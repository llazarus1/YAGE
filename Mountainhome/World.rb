require 'Camera'
require 'Terrain'
require 'TerrainBuilder'

class World < MHWorld
    def initialize(params={})
        @height = params[:height] || 33
        @width  = params[:width]  || 33
        @depth  = params[:depth]  || 17
        super(@width, @height, @depth)
    
        # Create a default camera
        @camera = Camera.new(self, 0)
        @camera.set_fixed_yaw(0, 0, 1)
        @camera.set_position(0.25*@width, 0.25*@height, 3*@depth)
        @camera.look_at(0.55*@width, 0.45*@height, 0)
        @pitch = 0
        @yaw = 0
        @movement = [0,0,0]
        
        # Create the terrain object
        self.terrain = Terrain.new(@width, @height, @depth)
        TerrainBuilder.add_layer(self.terrain,       1, 0.0, 0.8, 5000.0, 0.53)
        TerrainBuilder.composite_layer(self.terrain, 2, 0.4, 0.2, 5000.0, 0.35)

        self.populate
    end # def initialize

    def update(elapsed)
        sens_cap = 0.5
        pitch = [[@pitch*elapsed, sens_cap].min, -sens_cap].max
        yaw =   [[@yaw*elapsed,   sens_cap].min, -sens_cap].max
        @camera.rotate_on_axis(pitch, 1, 0, 0) if pitch != 0.0
        @camera.rotate_on_axis(yaw, 0, 0, 1) if yaw != 0.0
        @pitch = 0
        @yaw = 0

        move = @movement.collect {|elem| elem * elapsed}
        @camera.move_relative(*move)
    end

    def input_event(params={})
        case params[:type]
        when :move
            rotate_speed = -0.001
            @yaw = params[:x] * rotate_speed
            @pitch = params[:y] * rotate_speed
            return :handled
        when :keyboard
            movement_speed = 0.05
            case params[:key]
            when Keyboard.KEY_UP
                if params[:state] == :pressed or params[:state] == :typed
                    if params[:modifier] == 1
                        @movement[1] = movement_speed
                    else
                        @movement[2] = -movement_speed 
                    end
                else
                    @movement[1] = 0 if @movement[1] > 0
                    @movement[2] = 0 if @movement[2] < 0
                end
                return :handled
            when Keyboard.KEY_DOWN
                if params[:state] == :pressed or params[:state] == :typed
                    if params[:modifier] == 1
                        @movement[1] = -movement_speed
                    else
                        @movement[2] = movement_speed
                    end
                else
                    @movement[1] = 0 if @movement[1] < 0
                    @movement[2] = 0 if @movement[2] > 0
                end
                return :handled
            when Keyboard.KEY_LEFT
                if params[:state] == :pressed or params[:state] == :typed
                    @movement[0] = -movement_speed
                else
                    @movement[0] = 0 if @movement[0] < 0
                end
                return :handled
            when Keyboard.KEY_RIGHT
                if params[:state] == :pressed or params[:state] == :typed
                    @movement[0] = movement_speed
                else
                    @movement[0] = 0 if @movement[0] > 0
                end
                return :handled
            end
        end
        return :unhandled
    end
end
