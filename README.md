# g2802_array

1) verify .ino
2) cd Downloads
3) echo 'KERNEL=="ttyACM*", MODE="0666"' | sudo tee /etc/udev/rules.d/99-arduino.rules
4) echo 'KERNEL=="ttyUSB*", MODE="0666"' | sudo tee -a /etc/udev/rules.d/99-arduino.rules
5) sudo udevadm control --reload-rules && sudo udevadm trigger
6) upload .ino
7) run matlab script
8) run jupyter notebook
