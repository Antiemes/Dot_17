pio run
sudo avrdude -c dragon_dw -p attiny85 -U flash:w:.pioenvs/attiny85/firmware.hex
sleep 1
avarice --debugwire --dragon :4242 & sleep 1 ; netcat localhost 4242 & sleep 1 ; killall netcat ; killall avarice

