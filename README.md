# g2802_array

* Pin 1    GROUND
* Pin 2
* Pin 3    GROUND
* Pin 4
* Pin 5    GROUND
* Pin 6    HIGH -> LOW
* Pin 7    HIGH -> LOW
* Pin 8    GROUND
* Pin 9    GROUND
* Pin 10   PMOD 0
* Pin 11   GROUND
* Pin 12   
* Pin 13   GROUND
* Pin 14   PMOD 1
* Pin 15   GROUND
* Pin 16
* Pin 17   HIGH -- chip-select pin for SPI communication for both UDC chips
* Pin 18   PMOD 2
* Pin 19   HIGH (for TX, low for RX)
* Pin 20   
* Pin 21   LOW
* Pin 22   HIGH (enable H-pol channels)
* Pin 23
* Pin 24   GROUND
* Pin 25
* Pin 26   GROUND

1) verify .ino
2) cd Downloads
3) echo 'KERNEL=="ttyACM*", MODE="0666"' | sudo tee /etc/udev/rules.d/99-arduino.rules
4) echo 'KERNEL=="ttyUSB*", MODE="0666"' | sudo tee -a /etc/udev/rules.d/99-arduino.rules
5) sudo udevadm control --reload-rules && sudo udevadm trigger
6) upload .ino
8) run matlab script (load LUTs)
9) pins 6 & 7 to LOW
   
10) run overlay (jupyter notebook)
11) program device (vivado)
12) continue with jupyter notebook
