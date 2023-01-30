class Ledto
  def initialize
    @pin = 0
    
  end
  def testprint
    #gpio_set_level(@pin, 1)
    puts "GPS !!"
  end

  def serread
    ser = get_data
    
    puts "Received message 0: #{ser[0]}"
    puts "Received message 1: #{ser[1]}"
    puts "Received message 2: #{ser[2]}"
    puts "Received message 3: #{ser[3]}"
    puts "Received message 4: #{ser[4]}"
    puts "Received message 5: #{ser[5]}"
    puts "Received message 6: #{ser[6]}"
    puts "Received message 7: #{ser[7]}"
    puts "Received message 8: #{ser[8]}"
    
  end
end
