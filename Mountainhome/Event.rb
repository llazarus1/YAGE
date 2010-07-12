class Event < Hash
    def self.hash_attr(*names)
        names.each do |name|
            class_eval "def #{name}()     self[:#{name}]       end"
            class_eval "def #{name}=(val) self[:#{name}] = val end"
        end
    end

    def self.key_pressed(key, modifier = 0)
        self.new({:type => :keyboard, :state => :pressed, :key => key, :modifier => modifier})
    end

    def self.key_released(key, modifier = 0)
        self.new({:type => :keyboard, :state => :released, :key => key, :modifier => modifier})
    end

    def self.mouse_pressed(button, x, y)
        self.new({:type => :mouse, :state => :pressed, :button => button, :x => x, :y => y})
    end

    def self.mouse_released(button, x, y)
        self.new({:type => :mouse, :state => :released, :button => button, :x => x, :y => y})
    end

    def self.mouse_moved(absX, absY, relX, relY)
        self.new({:type => :move, :abs_x => absX, :abs_y => absY, :x => relX, :y => relY})
    end

    @@uppercase_punc = {
        Keyboard.KEY_1 => Keyboard.KEY_EXCLAIM,
        Keyboard.KEY_2 => Keyboard.KEY_AT,
        Keyboard.KEY_3 => Keyboard.KEY_HASH,
        Keyboard.KEY_4 => Keyboard.KEY_DOLLAR,
        Keyboard.KEY_5 => Keyboard.KEY_PERCENT,
        Keyboard.KEY_6 => Keyboard.KEY_CARET,
        Keyboard.KEY_7 => Keyboard.KEY_AMPERSAND,
        Keyboard.KEY_8 => Keyboard.KEY_ASTERISK,
        Keyboard.KEY_9 => Keyboard.KEY_LEFTPAREN,
        Keyboard.KEY_0 => Keyboard.KEY_RIGHTPAREN,

        Keyboard.KEY_MINUS => Keyboard.KEY_UNDERSCORE,
        Keyboard.KEY_EQUALS => Keyboard.KEY_PLUS,

        Keyboard.KEY_LEFTBRACKET => Keyboard.KEY_LCURLY,
        Keyboard.KEY_RIGHTBRACKET => Keyboard.KEY_RCURLY,
        Keyboard.KEY_BACKSLASH => Keyboard.KEY_PIPE,

        Keyboard.KEY_SEMICOLON => Keyboard.KEY_COLON,
        Keyboard.KEY_QUOTE => Keyboard.KEY_QUOTEDBL,

        Keyboard.KEY_COMMA => Keyboard.KEY_LESS,
        Keyboard.KEY_PERIOD => Keyboard.KEY_GREATER,
        Keyboard.KEY_SLASH => Keyboard.KEY_QUESTION,
    }
    
    @@printable = [Keyboard.KEY_BACKSPACE, Keyboard.KEY_TAB, Keyboard.KEY_CLEAR, Keyboard.KEY_RETURN,
        (Keyboard.KEY_SPACE..Keyboard.KEY_AT).to_a,
        (Keyboard.KEY_LEFTBRACKET..Keyboard.KEY_RCURLY).to_a].flatten

    ############################
    # Instance Stuff           #
    ############################

    hash_attr :key, :type, :state, :modifier    

    def initialize(hash)
        hash.each_pair {|key, value| self[key] = value }
    end

    def printable?
        @@printable.include?(self.key)
    end

    # either or both shift keys held?
    def shift_held?
        modifier == Keyboard.MOD_SHIFT or
        modifier == Keyboard.MOD_LSHIFT or
        modifier == Keyboard.MOD_RSHIFT
    end

    # Change the key to the correct shifted character
    def convert_shift
        return self unless shift_held?
        # revise uppercase punctuation
        self.key = @@uppercase_punc[self.key] if @@uppercase_punc[self.key]

        # revise uppercase letters
        self.key -= 32 if Keyboard.KEY_a <= self.key and self.key <= Keyboard.KEY_z
        return self
    end
end