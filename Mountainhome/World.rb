require 'Camera'
require 'TerrainBuilder'
require 'Reticle'

class Timer
    def start(name)
        @times << [name, Time.now]
    end

    def stop
        @times[-1][1] = Time.now - @times[-1][1]
    end

    def reset
        @times = Array.new
    end

    def to_s
        @times << ["total", @times.transpose[1].inject(0, &:+)]
        width = @times.transpose[0].collect(&:size).max + 1
        "\n" + @times.collect { |name, elapsed| "%-#{width}s #{elapsed}" % ["#{name}:"] }.join("\n")
    end
end

class TerrainVerificationDecorator
    def initialize(terrain)
        @array = Array.new(terrain.width) { Array.new(terrain.height) { Array.new(terrain.depth) { nil } } }
        @terrain = terrain
    end

    def verify
        $logger.info "Verifying world!"
        (@terrain.height - 1).downto(0) do |y|
            line = []
            (@terrain.width - 1).downto(0) do |x|
                z = @terrain.get_surface(x, y)
                line << "%2s [%-2s]" % [z, get_backup_surface(x, y)]
            end
            $logger.info line.join(" ")
        end

        (@terrain.height - 1).downto(0) do |y|
            line = []
            0.upto(@terrain.width - 1) do |x|
                z = @terrain.get_surface(x, y)
                line << "%2s" % [(z - get_backup_surface(x, y)).to_s]
            end
            $logger.info line.join("      ")
        end

        # $logger.info "Printing cross sections"
        # 0.upto(@terrain.height - 1) do |y|
        #     $logger.info "Printing cross section of height #{y}"
        #     (@terrain.depth - 1).downto(0) do |z|
        #         line = Array.new
        #         0.upto(@terrain.width - 1) do |x|
        #             line << "%2s [%2s,%2s,%2s]" % [@terrain.get_tile(x, y, z), x, y, z]
        #         end
        #         $logger.info line.join(" ")
        #     end
        # end
    end

    def get_core(x, y)
        @array[x][y].collect { |value| value || "-" }.join(", ")
    end

    def get_backup_surface(x, y)
        zLevel = -1
        @array[x][y].each_with_index do |type, index|
            zLevel = index if type.to_i > 0
        end
        zLevel
    end

    def set_tile_type(x, y, z, type)
        @terrain.set_tile_type(x, y, z, type)
        @array[x][y][z] = type
    end

    def method_missing(name, *args)
        @terrain.send(name, *args)
    end
    
end

class World < MHWorld
    attr_reader :builder_fiber
    attr_accessor :actors, :cameras, :active_camera
    
    def initialize(core, tile_types, action = :load, args={})
        super(core)

        case action
        when :empty
            if true
                width  = 3
                height = 3
                depth  = 2

                self.load_empty(width, height, depth, core)
                tile_types.each { |klass| terrain.register_tile_type(klass.new.material) }

                0.upto(width - 1) { |x| 0.upto(height - 1) { |y| terrain.set_tile_type(x, y, 0, 1) } }

                # 0.upto((width - 1)) do |a|
                #     0.upto((width - 1)) do |b|
                #         terrain.set_tile_type([a - b, 0].max, b, 0, 2)
                #         terrain.set_tile_type([a - b, 0].max, b, 1, 2)
                #     end
                # end

                # terrain.set_tile_type(2, 2, 0, 2)
                # terrain.set_tile_type(2, 2, 1, 2)

                terrain.set_tile_type(0, 0, 1, 2)
                terrain.set_tile_type(0, 1, 1, 2)
                terrain.set_tile_type(0, 2, 1, 2)

                terrain.set_tile_type(1, 2, 1, 2)
            else
                width  = 6
                height = 4
                depth  = 2

                self.load_empty(width, height, depth, core)
                tile_types.each { |klass| terrain.register_tile_type(klass.new.material) }
                0.upto(width - 1) { |x| 0.upto(height - 1) { |y| terrain.set_tile_type(x, y, 0, 2) } }
                0.upto(width - 1) { |x| 0.upto(height - 1) { |y| terrain.set_tile_type(x, y, 1, 2) } }
                terrain.set_tile_type(3, 3, 1, 0)
                terrain.set_tile_type(3, 2, 1, 0)
            end

            self.terrain.poly_reduction = true
            self.terrain.auto_update = true

            @builder_fiber = Fiber.new { true }
        when :generate
            width  = args[:width]  || 129
            height = args[:height] || 129
            depth  = args[:depth]  || 65

            self.load_empty(width, height, depth, core)
            tile_types.each { |klass| terrain.register_tile_type(klass.new.material) }

            # Generate a predictable world to see the effects of turning various terrainbuilder features on and off
            seed = rand(100000)

            # ATTN: Loch
            # seed = 66870 # REPROS THE HOLE IN THE WORLD BUG
            # Steps to repro:
            #  1) Generate a small world
            #  2) Run world.terrain.set_tile_type(0,1,5,0)
            #  3) HOLE!

            seed = 33843
            # seed = 99632 # Long poly reduction times for larger sizes.
            # seed = 67717 # SLOW
            # seed = 14012 # A neat world.
            # seed = 48103 # Used for benchmarking

            $logger.info "Building terrain with seed #{seed}"
            srand(seed)

            # Verify the integrity of the octree
            #TerrainBuilder.test_populate(terrain)
            #terrain.clear

            # Get the terrain object and install a special decorator to verify our results
            # if the map is small enough to make it feasible.
            terrain = self.terrain
            # if terrain.width < 32 && terrain.height < 32 && terrain.depth < 32
            #     terrain = TerrainVerificationDecorator.new(self.terrain)
            # end

            @timer = Timer.new
            @builder_fiber = Fiber.new do
                # Do the actual world generation and benchmark it as we go.
                $logger.info "Starting world generation:"
                $logger.indent

                @timer.reset
#                do_builder_step(:add_layer,          nil,  terrain, Bedrock, 0.0, 1.0, 5000.0, 0.55)
#                do_builder_step(:composite_layer,    nil,  terrain, Hardrock, 0.2, 0.4, 5000.0, 0.3 )
                do_builder_step(:add_layer,          nil,  terrain, 1, 0.0, 1.0, 5000.0, 0.55)
                do_builder_step(:composite_layer,    nil,  terrain, 2, 0.2, 0.4, 5000.0, 0.3 )
                do_builder_step(:shear,              nil,  terrain, 5, 1, 1)
                do_builder_step(:shear,              nil,  terrain, 2, 1, 1)
                do_builder_step(:generate_riverbeds, nil,  terrain, 1)
                do_builder_step(:average,            true, terrain, 2)
                #do_builder_step(:fill_ocean,         true, terrain, liquid_manager)
                @timer.to_s.split(/\n/).each { |line| $logger.info line }

                terrain.verify if terrain.respond_to?(:verify)

                $logger.info "World generation finished."
                $logger.unindent

                true # To indicate we're done.
            end
        when :load
            # TODO: The load has to be called outside of the fiber for now because the
            # dimensions are set based on the loaded data and the dimensions are ALSO
            # needed for setting up the camera. This means the information needs to be
            # available BEFORE the fiber executes. This is bad, though, because it means
            # we have dead time on the loading screen. This needs to be fixed so we have
            # an immediate jump to the loading screen without and camera exceptions.
            self.load(args[:filename]);
            self.terrain.poly_reduction = true
            self.terrain.auto_update    = true
            @builder_fiber = Fiber.new { true }
        end

        # Setup the cameras
        @cameras = []

        # Main camera
        maincam = IsoCamera.new("MainCamera", self)
        @cameras << maincam

        # Top-down camera
        topcam = TopCamera.new("TopDownCamera", self)
        @cameras << topcam

        # First-person camera
        fpcam = FirstPersonCamera.new("FirstPersonCamera", self, nil)
        @cameras << fpcam

        topcam.set_active

        @mouselook = false

        # And define some initial values.
        @pitch = 0
        @yaw = 0
        @movement = [0, 0, 0]

        $logger.info("World size: #{width}x#{height}x#{depth}")
    end

    def do_builder_step(name, final, *args)
        # This should work, but poly reduction is actually broken. Leaving this
        # here as a reminder.
        self.terrain.poly_reduction = final
        # self.terrain.poly_reduction = false
        self.terrain.auto_update    = false

        @timer.start(name.to_s)
        TerrainBuilder.send(name, *args)
        @timer.stop

        $logger.info("Step finished. Generating geometry.")
        @timer.start("Populate")
        self.populate()
        @timer.stop

        self.terrain.auto_update = final

        Fiber.yield
    end

    def update(elapsed)
        sensitivity = 1.0
        @active_camera.adjust_pitch(@pitch * sensitivity) if (@pitch != 0.0 && @active_camera.respond_to?(:adjust_pitch))
        @active_camera.adjust_yaw(  @yaw   * sensitivity) if (@yaw   != 0.0 && @active_camera.respond_to?(:adjust_yaw))
        @pitch = @yaw = 0

        move = @movement.collect {|elem| elem * elapsed}
        @active_camera.move_relative(*move) if @active_camera.respond_to?(:move_relative)

        @active_camera.update if @active_camera.respond_to?(:update)

        # update actors
        @actors.each { |actor|
            actor.update(elapsed)
        }
    end

    def toggle_mouselook
        @mouselook = !@mouselook
    end

    def input_event(event)
        case event
        when MouseMoved
            if @mouselook
                rotate_speed = -0.002
                @yaw   = event.relX * rotate_speed
                @pitch = event.relY * rotate_speed
                return :handled
            end
        when KeyPressed, KeyReleased
            movement_speed = 0.05
            case event.key
            when Keyboard.KEY_UP, Keyboard.KEY_w
                if event.state == :pressed
                    if event.shift_held?
                        @movement[2] = -movement_speed 
                    else
                        @movement[1] = movement_speed
                    end
                else
                    @movement[1] = 0 if @movement[1] > 0
                    @movement[2] = 0 if @movement[2] < 0
                end
                return :handled
            when Keyboard.KEY_DOWN, Keyboard.KEY_s
                if event.state == :pressed
                    if event.shift_held?
                        @movement[2] = movement_speed
                    else
                        @movement[1] = -movement_speed
                    end
                else
                    @movement[1] = 0 if @movement[1] < 0
                    @movement[2] = 0 if @movement[2] > 0
                end
                return :handled
            when Keyboard.KEY_LEFT, Keyboard.KEY_a
                if event.state == :pressed
                    @movement[0] = -movement_speed
                else
                    @movement[0] = 0 if @movement[0] < 0
                end
                return :handled
            when Keyboard.KEY_RIGHT, Keyboard.KEY_d
                if event.state == :pressed
                    @movement[0] = movement_speed
                else
                    @movement[0] = 0 if @movement[0] > 0
                end
                return :handled
            end
        end
        return :unhandled
    end

    # The World is in charge of creating Actors.
    def create_actor(klass, name, model, material, pos)
      # Only actors can be created with Entities currently.
      
      actor = klass.new
      unless actor.is_a?(Actor)
        $logger.error("Not an Actor class: #{klass}")
        return
      end

      actor.name = name
      actor.position = pos
      actor.world = self

      $logger.info("Creating actor #{name}")
      
      # When an Actor is created, a corresponding Entity is created in C and associated with the Actor.
      actor.entity = create_entity(name, model, material, pos[0], pos[1], pos[2])
      
      # actors are tracked in Ruby by World
      @actors << actor
    end

    # Will need to dereference the Actor and delete Entity associated.
    def delete_actor(actor)
      original_size = @actors.size
      @actors.delete(actor)
      unless @actors.size == original_size - 1
        $logger.error("Error deleting actor #{actor}")
        return
      end
      # Entity deletion needs to happen in C++.
      delete_entity(actor.name)
    end
end
