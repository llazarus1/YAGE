require 'UIElement'
require 'LookNFeel'
#require 'Reticule'

class UIManager < MHUIManager
    attr_accessor :active_element, :focus_override, :mouse
    def initialize
        @active = false
        @active_element = nil
        @focus_override = nil

        # For now, create a default LookNFeel
        @looknfeel = LookNFeel.new

        # AJEAN - Temporary code
        self.root = create(UIElement)
        #@mouse    = create(Mouse, {:parent => self.root})

        @cursor = true

        #@persistent_elems = [@mouse]
        @persistent_elems = []
    end

    # This call is for menu builders, and is used to clear everything except the root and mouse elements
    def clear_elements(clear_all = false)
        self.root.delete_children if self.root
        @persistent_elems.each { |elem| self.root.add_child(elem) }
    end
    
    def teardown
        $logger.info "Tearing down UIManager"
    end

    def resize(width, height)
        self.root.set_dimensions(0, 0, width, height)
        self.root.compute_dimensions
    end

    def update(elapsed)
        # Update elements
        # TODO - Fix this when UIElements are a thing again
        #self.root.update(elapsed)
    end

    def toggle_cursor
        @cursor = !@cursor
        if @cursor
            self.root.add_child(@mouse)
            @persistent_elems << @mouse
        else
            self.root.cull_child(@mouse)
            @persistent_elems.delete(@mouse)
        end
    end

    def input_event(event)
        # TODO - Fix this once the mouse is back in action
        return :unhandled

        case event
        when MousePressed
            # Set the active element to the highest depth UI element, or nil.
            @active_element = element_at(@mouse.x, @mouse.y)
            if @active_element
                $logger.info "User clicked on UIElement #{@active_element.inspect}"
                # Sliders need to know the x value of the mouse until the mouse button is released.
                return @active_element.on_click { @mouse.x } if @active_element.respond_to?(:on_click)
            else
                kill_element(@selection) unless @selection.nil?
                @selection = create(Pane, {:anchor => [@mouse.x, @mouse.y], :parent => self.root})
            end
        when MouseReleased
            if @selection
                kill_element(@selection) unless @selection.nil?
                @selection = nil
            end
            if @active_element and @active_element.respond_to?(:on_release)
                @active_element.on_release
            end
        when MouseMoved
            if @cursor
                @mouse.x = [[@mouse.x + event.relX, 0].max, self.width ].min
                @mouse.y = [[@mouse.y - event.relY, 0].max, self.height].min
            end
            @selection.resize(@mouse.x-@selection.x, @mouse.y-@selection.y) unless @selection.nil?

            # Moving the mouse pointer shouldn't kill any other effects of mouse movement.
            return :unhandled
        when KeyPressed
            if @active_element and @active_element.respond_to?(:input_event)
                return @active_element.input_event(event)
            end
        end
        return :unhandled
    end

    def kill_element(elem)
        $logger.info "Culling #{elem}"
        self.root.cull_child(elem)
    end

    # Find the topmost menu element at [x,y]
    def element_at(x, y)
        elems = self.root.elements_at(x,y,0)
        topmost = {:element => nil, :d => -1}
        elems.each do |element|
            topmost = element if element[:element] and (topmost[:d] < element[:d])
        end
        return topmost[:element] if @focus_override.nil? or @focus_override.include?(topmost[:element])
    end

    # Element creation method
    # Creates an element of type klass, using the args hash to configure it, and possibly passing it a block
    def create(klass, args={}, material=nil, &block)
        # Create the UIElement
        object = klass.new() { |*params| block.call(*params) if block_given? }
        args.each_pair { |k,v| object.send("#{k}=", v) }

        # AJEAN - Since UIElements no longer create themselves, we can have the UI compute their pixel dimensions
        #  before passing them to the LookNFeel, which does the actual renderable creation
        # Compute pixel dimensions for passing to the LookNFeel
        dims = []
        object.ldims.each_with_index do |dim, index|
            dims << if index.even?
                (dim * (self.width.to_f  / $max_dim)).to_i
            else
                (dim * (self.height.to_f / $max_dim)).to_i
            end
        end

        # Call C object bindings
        object.x = dims[0]
        object.y = dims[1]
        # Only required for Ruby
        object.w = dims[2]
        object.h = dims[3]

        # Call on the looknfeel
        @looknfeel.prepare_element(object, self)

        object
    end
end
