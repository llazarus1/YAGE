class UIElement < MHUIElement
    def initialize(name, manager, mat, text, args={})
        @children = []
        @clickable = args[:clickable] || false
        @update_proc = args[:update_proc] || nil

        super(name, manager, mat, text)
    end
    
    def set_dimensions(x,y,w,h)
        super(x,y,w,h)
    end
    
    def add_child(child)
        @children << child
        super(child)
    end

    def update(elapsed)
        @update_proc.call if @update_proc
        @children.each { |c| c.update(elapsed) }
    end

    def cull_child(child)
        if @children.include? child 
            @children.delete(child) 
            super(child)
        else
            @children.each { |e_child| e_child.cull_child(child) }
        end
    end

    def elements_at(x, y, d)
        collisions = []
        @children.each do |child|
            child.elements_at(x, y, d+1).each { |subcoll| collisions << subcoll }
        end
        if @clickable and 
           (x >= self.x) and (x <= (self.x + self.w)) and
           (y >= self.y) and (y <= (self.y + self.h))
            return collisions << {:element => self, :d => d}
        else
            return collisions
        end
    end
end

class InputField < UIElement
    def push_char(char)
        self.text = (self.text + char)
    end

    def pop_char
        self.text = self.text[0..-2]
    end
end

class Console
    def initialize(manager, eval_proc)
        @p_eval = eval_proc
        @toggled = false

        buffer_length = 15
        @history = Array.new(buffer_length, ">")

        # Create the menu elements
        @window = nil
        hist_upd = Proc.new { @window.text = @history.join("\n") }
        @window  = manager.add_element("console_window", 5, -30, manager.width-10, 220,
                                           {:mat => "t_grey", :update_proc => hist_upd})
        @input_field = manager.add_element("console_input",  5, -10, manager.width-10, 20, 
                                           {:mat => "t_grey", :element_type => InputField})
        @window.set_offset(0,-205)
        @input_field.set_offset(0,-5)
        @input_field.set_border(2)
    end

    def input_event(args={})
        case args[:key]
        when Keyboard.KEY_BACKQUOTE
            return :unhandled
        when Keyboard.KEY_RETURN
            # Call the proc
            result = @p_eval.call(@input_field.text)
            # Place the command in history
            @history = [result, @input_field.text] + @history[0..-3]
            @input_field.text = ""
            return :handled
        when Keyboard.KEY_BACKSPACE
            @input_field.pop_char
            return :handled
        else
            @input_field.push_char([args[:key]].pack("C"))
            return :handled
        end

        return :unhandled
    end

    def toggle
        if @toggled
            [@input_field, @window].each { |e| e.move_relative(0, -220) }
        else
            [@input_field, @window].each { |e| e.move_relative(0, 220) }
        end
        @toggled = !@toggled
    end

    def teardown(manager)
        manager.kill_element(@input_field)
        @input_field = nil
    end
end
