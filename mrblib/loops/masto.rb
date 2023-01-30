led1 = Led.new(18)
led2 = Led.new(19)
gps = Ledto
while true
  #co2 = co2.concentrate
  #puts "CO2: #{co2}"
  #gps = gps.concentrate
  
  led1.turn_on
  sleep 1
  led1.turn_off
  sleep 1

  led2.turn_on
  sleep 1
  led2.turn_off
  sleep 1
end