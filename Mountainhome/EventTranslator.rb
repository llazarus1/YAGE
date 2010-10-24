#!/usr/bin/ruby

require 'Event'

# ActionRegistration
class EventTranslator
    def initialize
        @event_to_name  = {}
        @name_to_action = {}
    end

    def register_action(name, &block)
        $logger.info("register_action: #{name} #{block.inspect}")
        @name_to_action[name] = block
    end

    def register_event(name, event)
        $logger.info("register_event: #{name} #{event}")
        @event_to_name[event] = name
    end

    def register_event_to_action(event, name, &block)
        @event_to_name[event] = name
        @name_to_action[priority][name] = block
    end

    def call_action(name)
        if (action = @name_to_action[name]) and
           (action.is_a?(Proc))
            action.call
            return :handled
        end
        :unhandled
    end

    def input_event(event)
        if name = @event_to_name[event]
            call_action(name)
        end
    end
end
