class UIElement < MHUIElement
    attr_reader :name
    def initialize(name, manager, mat, font, text, args={})
        @update_proc    = args[:update_proc]
        @name           = name

        # Pass the parameters to the C Object
        super(name, manager, mat, font, text)

        if args[:parent]
            args[:parent].add_child(self)
        end
    end

    def update(elapsed)
        @update_proc.call if @update_proc
        each_child do |child|
            child.update(elapsed)
        end
    end

    def elements_at(x, y, d)
        collisions = []

        each_child do |child|
            child.elements_at(x, y, d+1).each { |subcoll| collisions << subcoll }
        end
        elem_x = self.x + self.x_offset
        elem_y = self.y + self.y_offset
        if @clickable and 
           (x >= elem_x) and (x <= (elem_x + self.w)) and
           (y >= elem_y) and (y <= (elem_y + self.h))
            return collisions << {:element => self, :d => d}
        else
            return collisions
        end
    end
end

class Clickable < UIElement
    attr_accessor :clickable

    def initialize(name, manager, mat, font, text, args={}, &block)
        @clickable = true
        @click_proc = block || Proc.new { $logger.info "No click_proc specified for #{self.inspect}" }
        super(name, manager, mat, font, text, args)
    end

    def on_click(type = :click, &block)
        @click_proc.call(type) { yield if block }
    end
end

class InputField < UIElement
    def push_char(char, shifted = false)
        char = $uppercase_punc[char] if shifted and $uppercase_punc[char]
        self.text = (self.text + [char].pack("C"))
    end

    def pop_char
        self.text = self.text[0..-2]
    end
end

# This is used to create an invisible element whose dimensions don't matter
# We use this to group elements together for easy deletion
class Invisible < UIElement
    def initialize(name, manager, args={})
        super("invisible_#{name}", manager, "", "", "", args)
    end
end

class Selectable < Clickable
    def initialize(name, manager, text, x, y, w, h, args={}, &block)
        super("button_#{name}", manager, args[:mat] || "t_grey", "", text, args) { yield }

        set_dimensions(x, y, w, h)
    end
end

class Button < Selectable
    def initialize(name, manager, text, x, y, w, h, args={}, &block)
        t_dims = [manager.text_width(text)/2, manager.text_height/2]
        button_pos = [x - t_dims[0], y - t_dims[1]]
        button_offset = [x - (w/2) - button_pos[0], y - (h/2) - button_pos[1]]

        super(name, manager, text, button_pos[0], button_pos[1], w, h, args) { yield }

        set_offset(button_offset[0], button_offset[1])
        set_border(2)
    end
end

class Pane < UIElement
    def initialize(name, manager, x, y, w, h, args={})
        super("pane_#{name}", manager, "t_grey", "", "", args)

        set_dimensions(x,y,w,h)
    end
end

class Text < UIElement
    def initialize(name, manager, text, x, y, args={})
        t_dims = [manager.text_width(text), manager.text_height]
        t_pos = [x - (t_dims[0]/2), y - (t_dims[1]/2)]

        super("title_#{name}", manager, "", args[:font] || "", text, args)

        set_dimensions(t_pos[0], t_pos[1], t_dims[0], t_dims[1])
    end
end

class Image < UIElement
    def initialize(name, manager, mat, x, y, w, h, args={})
        i_pos = [x - (w/2), y - (h/2)]

        super("image_#{name}", manager, mat, "", "", args)

        set_dimensions(i_pos[0], i_pos[1], w, h)
    end
end

class Mouse < UIElement
    def initialize(manager, args={})
        super("mouse", manager, "cursor", "", "")

        set_dimensions(0,0,14,21)
        set_offset(0,-21)
        always_on_top
    end
end

# Slider tracks a single value from 0.0 to 1.0, passing it to the block as an argument when it changes
class Slider < Selectable
    def initialize(name, manager, def_value, x, y, w, h, args = {}, &block)
        @value  = def_value || 0.5
        @moving = false

        # This proc is used to obtain whatever value is using to modify the position of the slider
        @source   = Proc.new { $logger.info "No data source specified for slider #{name}" }

        # This proc is used to pass the slider's value back to whatever object cares
        @tracker  = block     || Proc.new { nil }

        super("slider_#{name}", manager, "", x, y, w, h, args.merge!(:mat => ""))

        Pane.new("sliderbar_#{name}", manager, x, y+(h/2)-2, w, 4, {:parent => self})
        @tab = Pane.new("sliderpane_#{name}", manager, x+(w*@value), y, 10, h, {:parent => self})
        @tab.set_border(2)
    end

    def update(elapsed)
        if @moving
            # Obtain the source value and keep the slider within its boundaries
            @tab.x = [[@source.call, self.x].max, self.x+self.w].min - (@tab.w/2.0)
            @value = (@tab.x + (@tab.w/2.0) - self.x) / self.w
            # Pass the new slider value back
            @tracker.call(@value)
        end
    end

    # Since the slider needs to follow the mouse, we overload the on_click method to deal with both mouse
    #  presses *and* releases, using a block to track whatever parameter controls the slider (typical use: mouse.x)
    def on_click(type, &block)
        case type
        when :click
            @moving  = true
            (@source = block) if block
        when :release
            @moving = false
            @source = Proc.new { $logger.info "No tracker specified for slider #{name}" }
        else
            $logger.info "No event type specified for slider - needs either :click or :release"
        end
    end
end

# DropDown is a button that the user clicks on which generates a drop-down menu of choices.
# The drop-down menu is generated and handled by the list_proc, which should take the DropDown as a parameter
class DropDown < Selectable
    def initialize(name, manager, def_value, x, y, w, h, args={}, &block)
        @state     = :closed
        @list_proc = block || Proc.new { $logger.info "No list proc specified for DropDown element #{name}" }

        super("dropdown_#{name}", manager, def_value, x, y, w, h, args) { toggle }

        set_border(2)
    end

    def selected(item)
        self.text = item
        toggle
    end

    def toggle
        case @state
        when :closed
            @state = :open
            @list_proc.call(self)
        when :open
            @state = :closed
            self.cull_children
        end
    end
end

class Console < InputField
    def initialize(manager, args={}, &block)
        @manager = manager

        @p_eval = block
        @toggled = false

        buffer_length = 15
        @history = Array.new(buffer_length, ">")

        # This element will be the input field, with the history buffer as its child
        super("console", manager, "t_grey", "", "", args)

        set_dimensions(5, -10, manager.width-10, 20)
        set_offset(0,-5)
        set_border(2)

        @window = nil
        hist_upd = Proc.new { @window.text = @history.join("\n") }
        @window = Pane.new("console_history", manager, 5, -30, manager.width-10, 220, {:update_proc => hist_upd})
        @window.set_offset(0,-205)
        add_child(@window)
    end

    def input_event(event={})
        if @toggled
            case event[:key]
            when Keyboard.KEY_BACKQUOTE
                toggle
                return :handled
            when Keyboard.KEY_RETURN
                # Call the proc
                result = call(self.text)
                # Place the command in history
                @history = [result, @window.text] + @history[0..-3]
                self.text = ""
                return :handled
            when Keyboard.KEY_BACKSPACE
                pop_char
                return :handled
            when Keyboard.KEY_a..Keyboard.KEY_z
                if shifted?(event)
                    event[:key] -= 32
                end
                push_char(event[:key])
                return :handled
            # everything except KEY_PAUSE
            when Keyboard.KEY_BACKSPACE, Keyboard.KEY_TAB, Keyboard.KEY_CLEAR, Keyboard.KEY_RETURN, Keyboard.KEY_ESCAPE,
                 Keyboard.KEY_SPACE..Keyboard.KEY_DOLLAR,
                 Keyboard.KEY_AMPERSAND..Keyboard.KEY_AT,
                 Keyboard.KEY_LEFTBRACKET..Keyboard.KEY_UNDERSCORE
                push_char(event[:key], shifted?(event))
                return :handled
            else
            end
        else 
        end
        return :unhandled
    end

    def toggled?
        @toggled
    end

    def toggle
        if @toggled
            [self, @window].each { |e| e.move_relative(0, -220) }
            @manager.active_element = nil
        else
            [self, @window].each { |e| e.move_relative(0, 220) }
            @manager.active_element = self
        end
        @toggled = !@toggled
    end

    def call(local_text)
        begin
            @p_eval.call(local_text)
        rescue Exception => e
            e.message
        end
    end
end
