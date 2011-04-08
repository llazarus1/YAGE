$max_dim = 32

class UIElement
    attr_accessor :x, :y, :w, :h
    attr_accessor :lay_dims, :lay_pos

    def initialize(args={})
        self.x ||= 0
        self.y ||= 0
        self.w ||= 0
        self.h ||= 0

        self.visible = true
    end

    attr_writer :on_top
    def on_top?; @on_top || false; end

    def visible?() self.visible end
    attr_reader :visible
    def visible=(val) @visible=val; self.dirty = true end

    # The dirty flag is used to inform the UIManager that an element's
    #  renderables should be recreated (as well as any dependent elements
    #  that were created by the looknfeel)
    attr_writer :dirty
    def dirty?; @dirty || false; end

    def update_derived_values(parent=nil)
        # Update the absolute position
        @abs_pos = parent.nil? ? [self.x, self.y] : [parent.x+self.x, parent.y+self.y]

        # Update the derived values of the dependents
        self.dependents.each do |dependent|
            dependent.update_derived_values(self)
        end

        # Update the model matrices of the renderables
        self.renderables.each do |renderable|
            renderable.set_translation_matrix(@abs_pos[0], @abs_pos[1], 0)
        end
    end

    # Dependent management
    def dependents;           @dependents ||= [];           end
    def add_dependent(dep);   self.dependents << dep;       end
    def add_dependents(deps); self.dependents.concat(deps); end
    def delete_dependents;    @dependents = [];             end

    # Renderable management
    def get_renderables(top=[])
        ret = []
        self.dependents.each do |dep|
            ret += dep.get_renderables(top)
        end
        if on_top?
            top.concat(self.renderables)
            ret
        else
            self.renderables + ret
        end
    end
    def renderables;                @renderables ||= [];            end
    def add_renderable(renderable); self.renderables << renderable; end
    def add_renderables(rends);     self.renderables.concat(rends); end
    def delete_renderables;         @renderables = [];              end

    def inspect
        super+":"+[self.x,self.y,self.w,self.h].inspect
    end
end
class UIPane < UIElement
    def update_derived_values(parent=nil)
        super(parent) unless parent.nil?
        self.children.each do |child|
            child.update_derived_values(parent)
        end
    end

    # Renderable management
    def get_renderables(top=[])
        ret = []
        self.children.each do |child|
            ret += child.get_renderables(top)
        end
        super(top) + ret
    end

    # Child management
    def children;         @children ||= [];       end
    def add_child(child); self.children << child; end
    def delete_children;  @children = [];         end
    def delete_child(elem)
        if self.children.delete(elem).nil?
            self.children.each { |child| child.delete_child(elem) if child.respond_to?(:delete_child) }
        end
    end
end

class Label < UIElement
    attr_accessor :color
    attr_writer :text
    def text; @text || ""; end
    def inspect; super + " " + self.text.inspect; end
end
class Title < Label; end

class InputField < UIElement
    attr_writer   :text
    attr_accessor :on_update, :on_return

    def initialize(args={}, &block)
        @on_update = block if block_given?
        super(args)
    end

    def text; @text || ""; end
    def text=(value)
        @text = value.to_s
        self.on_update.call(@text) unless self.on_update.nil?
        self.dirty = true
    end

    def on_click(event)
        Event.add_listeners(self)
    end

    def input_event(event)
        if event.type == :mouse && event.state == :pressed
            unless event.x > self.x && event.x < self.x + self.w &&
                   event.y > self.y && event.y < self.y + self.h
                Event.remove_listeners(self)
            end
        end

        return if event.type != :keyboard || event.state != :pressed

        if event.key == Keyboard.KEY_RETURN
            self.on_return.call(self.text) unless self.on_return.nil?
        elsif event.key == Keyboard.KEY_ESCAPE
            Event.remove_listeners(self)
        elsif event.key == Keyboard.KEY_BACKSPACE
            self.text = self.text.chop
        else
            self.text += event.ascii
        end
    end
end

class Button < UIElement
    attr_accessor :text
    attr_writer :on_click

    def initialize(args={}, &block)
        super(args)
        self.on_click = block if block_given?
    end

    def on_click(event, &block)
        @on_click.call(event) { yield if block_given? } unless @on_click.nil?
    end

    def inspect; super + " " + self.text.inspect; end
end
class InvisibleButton < Button; end
class Link < Button; end

class Slider < UIElement
    attr_accessor :cursor_pos,
                  :current_value,
                  :slider_values

    attr_writer :on_set

    def initialize(args={}, &block)
        super(args)
		self.on_set = block if block_given?
    end

    attr_writer :continuous
    def continuous() @continuous || false end

    def on_click(event)
        Event.add_listeners(self)
        self.move_slider_to(event.x - self.x, event.y - self.y)
    end

    def input_event(event)
        if event.type == :move
            self.move_slider_to(event.absX - self.x, event.absY - self.y)
        elsif event.type == :mouse && event.state == :released
            Event.remove_listeners(self)
        end
    end

    def move_slider_to(x, y)
        self.cursor_pos = [x, y]
        self.dirty = true
    end

	def current_value=(val)
		@on_set.call(val) if @on_set
		@current_value = val
	end
end

class Mouse < UIElement; end

class Console < InputField
    attr_writer :command_history
    attr_writer :history_buffer

    def initialize(args={}, &block)
        super(args) {}
        self.visible = false

        @eval_proc = block if block_given?
        self.on_return = Proc.new { evaluate_command }
    end

    def command_history() @command_history ||= [] end
    def history_buffer()  @history_buffer  ||= [] end

    def evaluate_command
        self.command_history << self.text

        new_buffer = [self.text]
        result = begin
            @eval_proc.call(self.text)
        rescue Exception => e
            e.message
        end
        new_buffer.concat(result.to_s.split(/\n/).collect { |text| "=> #{text}" })

        self.history_buffer.concat(new_buffer)

        self.text  = ""
        self.dirty = true
    end
end
