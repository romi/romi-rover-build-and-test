
Enabling serial on the Pi:

https://www.framboise314.fr/le-port-serie-du-raspberry-pi-3-pas-simple/
https://www.abelectronics.co.uk/kb/article/1035/serial-port-setup-in-raspberry-pi-os



1)--------------------
In /boot/config.txt, add:

  dtoverlay = pi3-disable-bt

In /boot/cmdline.txt, remove:
  console=serial0,115200 

2)--------------------
sudo raspi-config
Select “3 Interface Options”
Select “P6 Serial Port”
Step 4 - A screen will ask you if you would like a login shell to be accessible over serial.  Select No.
...







# Test 1, 05-test-serial-camera-1

This test requires that the Raspberry Pi has been configured to make
the hardware serial available for applications (see documentation in
libromi/README.md).

This test also requires the use of an Arduino Due.

It should be connected as shown in the image below:

![Wiring](wiring.jpg)


* Rpi Pin 6 -> Due Pin GND 
* Rpi Pin 8 (TX) -> Due Pin 19 (RX)
* Rpi Pin 10 (RX) -> Due Pin 18 (TX)

For the pins on the Raspberry Pi, check https://pinout.xyz

On the Due, upload the DueSerial sketch. On the PI, start
05-test-serial-camera-1. The Due sends a counter to the Pi, The Pi
prints the value of the counter to the console.

This test can be performed both on the RPi4 and the RPi Zero W.


# Test 2, 05-test-serial-camera-2

The second application tests the serial communication between the RPi4
and RPi Zero W. 
