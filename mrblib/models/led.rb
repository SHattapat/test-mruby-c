class Led
    def initialize(pin)
      @pin = pin
      gpio_init_output(@pin)
      turn_off
    end
  
    def turn_on
      gpio_set_level(@pin, 1)
      puts "turned on"
    end
  
    def turn_off
      gpio_set_level(@pin, 0)
      puts "turned off"
    end
  end