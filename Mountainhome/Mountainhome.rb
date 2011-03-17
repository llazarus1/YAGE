#############################
# Built in class extensions #
#############################

class Array
    def piecewise_add(array)
        if array.size != self.size
            raise ArgumentError, "Array sizes #{[array.size,self.size].inspect} don't match."
        else
            self.zip(array).map { |i,j| i+j }
        end
    end
end

class Symbol
    def constantize () self.to_s.constantize end
    def camelize    () self.to_s.camelize    end
end

class String
    def constantize
        name = self.camelize
        Object.const_defined?(name) ?
            Object.const_get(name) :
            Object.const_missing(name)
    end

    def camelize
        self.gsub(/\/(.?)/) { "::#{$1.upcase}" }.gsub(/(?:^|_)(.)/) { $1.upcase }
    end
end

class Object
    def self.delegate_to(obj, *methods)
        methods.each do |method|
            class_eval %{
                def #{method}(*args)
                    @#{obj}.#{method}(*args)
                end
            }
        end
    end
end

#
# Module that, when extended, will track all children of the object that are created after
# this module has been extended. Note that we have to use extend, because the included and
# inherited callbacks don't play well with the 'super' call otherwise. Basically if we
# want to allow children to have their own versions of these, we have to use extend (at
# least that's what my testing seemed to indicate).
#
module RecordChildren
    def inherited(klass)
        self.add_child(klass)
        klass.extend(RecordChildren)
    end

    def included(klass)
        self.add_child(klass)
        klass.extend(RecordChildren)
    end

    def add_child(klass, include_ancestors = true)
        @children ||= []

        unless @children.include?(klass)
            @children << klass

            if include_ancestors
                self.ancestors.each do |ancestor|
                    if ancestor.kind_of?(RecordChildren)
                        ancestor.add_child(klass, false)
                    end
                end
            end
        end
    end

    def ancestors(type = :all)
        ancestors = super()

        case type
        when :all     then ancestors
        when :classes then ancestors.select { |a| a.kind_of?(Class) }
        when :modules then ancestors.reject { |a| a.kind_of?(Class) }
        else nil
        end
    end

    def children(type = :all)
        @children ||= []
        case type
        when :all     then @children
        when :classes then @children.select { |a| a.kind_of?(Class) }
        when :modules then @children.reject { |a| a.kind_of?(Class) }
        else nil
        end
    end
end

########################
# Mountainhome modules #
########################
module MountainhomeTypeModule
    extend RecordChildren

    def self.included(base)
        class << base
            def class_attributes()     @attributes ||= {} end
            def class_attributes=(val) @attributes = val end

            def base_class()     @base_class ||= nil end
            def base_class=(val) @base_class = val end

            def manager()     @manager ||= nil end
            def manager=(val) @manager = val end

            def uses(modules)
                modules.each do |mod|
                    self.module_eval { include mod.constantize }
                end
            end

            def is_an(modules) is_a(modules) end

            def is_a(modules)
                modules.each do |mod|
                    mod = "#{mod}_module".constantize
                    self.instance_eval { include(mod) }

                    # Make sure we give preference to the subclass's keys and values.
                    self.class_attributes = mod.class_attributes.merge(class_attributes) if mod.respond_to?(:class_attributes)

                    # And don't forget to copy over the base type (though only do it if it's been set).
                    self.base_class = mod.base_class if mod.respond_to?(:base_class) && mod.base_class

                    # Also copy the manager
                    self.manager = mod.manager if mod.respond_to?(:manager) && mod.manager
                end
            end

            def has_attributes(*attrs)
                attrs.each do |attribute|
                    class_attributes[attribute] = nil
                end
            end

            def attribute_values(pairs)
                pairs.each do |attribute, value|
                    unless class_attributes.has_key?(attribute)
                        raise RuntimeError, "Cannot set value for undefined attribute: #{attribute}"
                    end

                    class_attributes[attribute] = value
                    if value.kind_of?(Proc)
                        define_method(attribute) { instance_eval(&@attributes[attribute]) }
                    else
                        define_method(attribute) { @attributes[attribute] }
                        define_method("#{attribute}=") { |value| @attributes[attribute] = value }
                    end
                end # pairs.each
            end # attribute_values
        end # class << base

        super

    end # self.included
end # module

module InstantiableModule
    def self.included(base)
        # Provide a way to get the instantiable class without using String ops
        class << base
            attr_accessor :inst_class
        end

        name = base.name.gsub(/Module$/, '')

        # Error out if no base type has been specified.
        if base.base_class.nil?
            raise RuntimeError, "Module '#{base}' does not have a defined base class."
        end

        Object.class_eval %{
            class #{name} < #{base.base_class}
                include #{base}
                # This will make sure MountainhomeObject's initialize will properly call the correct class_attributes method.
                def self.class_attributes
                    #{base}.class_attributes
                end
            end
        }

        base.inst_class = name.constantize
    end
end

###########################
# Mountainhome base types #
###########################
#
# Contains logic specific to all instantiable objects, as opposed to
# MountainhomeTypeModule, which contains logic to include in non-modules, instead.
#
module MountainhomeObjectModule
    include MountainhomeTypeModule
    def verify_attributes_are_filled_in
        nil_attrs = []
        self.class.class_attributes.each { |k, v| nil_attrs << k if v.nil? }
        return if nil_attrs.size == 0
        raise RuntimeError, "Cannot make instance of #{self.class}! The " +
            "following attributes are undefined:\n  #{nil_attrs.join("\n  ")}"
    end

    def eval_attributes
        map = Hash.new
        @attributes.keys.each { |key| map[key] = self.send(key) }
        map
    end

    def initialize(*args)
        verify_attributes_are_filled_in
        @attributes = self.class.class_attributes.dup
        super(*args)
    end
end

require 'Jobs'

# Deal with offsets to the graphics positioning.
# self.position, x, y and z are all in 'game' coordinates.
# set_position translates this into 'graphics' coordinates
# when informing an entity.
module TranslatePosition
    attr_reader :x, :y, :z

    # Set full position for every change.
    def x=(x) set_position( x,@y,@z); end
    def y=(y) set_position(@x, y,@z); end
    def z=(z) set_position(@x,@y, z); end

    attr_accessor :x_offset, :y_offset, :z_offset

    # By default, offset to the center of the tile.
    def x_offset() @x_offset || 0.5; end
    def y_offset() @y_offset || 0.5; end
    # And floating above (0.4 works well for spheres.)
    def z_offset() @z_offset || 0.4; end

    def set_position(x, y, z)
        @x = x
        @y = y
        @z = z
        # Can be included in MHActor (an entity) or an object with an @entity.
        if self.is_a?(MHActor)
            super(@x + self.x_offset, @y + self.y_offset, @z + self.z_offset)
        elsif @entity
            @entity.set_position(@x + self.x_offset, @y + self.y_offset, @z + self.z_offset)
        end
    end

    def position=(position)
        set_position(*position) if position.is_a?(Array) and position.size == 3
    end

    def position
        [@x, @y, @z]
    end
end

class Actor < MHActor
    include MountainhomeObjectModule
    include TranslatePosition

    attr_accessor :name

    def to_s; @name; end
end

class Tile
    include MountainhomeObjectModule
end

#######################
# And some setup code #
#######################
require 'Event'

require 'LoadingState'
require 'GameState'
require 'MenuState'

require 'PlantManager'

#########################################################
# Static class to track descriptions and their managers #
#########################################################

class MountainhomeDSL
    @managers = Hash.new

    def self.register_manager(klass)
        @managers[klass] ||= klass.new
    end

    def self.managers
        @managers ||= Hash.new
    end

    def self.describe(name, options = {}, &block)
        # Create the class.
        name = "#{name}_module"
        Object.class_eval "module #{name.camelize}; include MountainhomeTypeModule; end"
        $logger.info("Creating #{name}")
        klass = name.constantize

        # Extend the proper modules.
        klass.uses(options[:uses]) if options[:uses]
        klass.is_a(([options[:is_a]] + [options[:is_an]]).flatten.compact)

        # Set the base type if we need to.
        klass.base_class = options[:base] if options[:base]

        # Evaluate the block properly.
        klass.instance_eval(&block) if block_given?

        # Register the manager.
        if options[:managed_by]
            klass.manager = options[:managed_by]
            self.register_manager(klass.manager)
        end

        # Register the module with its manager, which may have been set from a superclass.
        if klass.manager && (klass.include? InstantiableModule)
            self.managers[klass.manager].register(klass.inst_class)
        end

        klass
    end
end

require 'Descriptions'

##################
# Global Objects #
##################

# MHCore objects cannot go out of scope.
$mhcore = MHCore.new
# Disable the menu until Andrew can get the menus working again.
$mhcore.register_state(MenuState.new($mhcore),    "MenuState")
$mhcore.register_state(LoadingState.new($mhcore), "LoadingState")
$mhcore.register_state(GameState.new($mhcore),    "GameState")
$mhcore.set_state("MenuState")
# $mhcore.set_state("LoadingState", :empty)

# $mhcore.set_state("LoadingState", :generate, {:width => 9, :height => 9, :depth => 9})
