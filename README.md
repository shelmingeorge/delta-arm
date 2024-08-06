Code for robo-arm project.

Currently is in progress. 

I am making 3-axis robo-arm with these modules:
two Nema 17 23 mm stepping motors;
Nema 17 40 mm stepping motor;
SG90 servo motor;
three A4988 motor encoders;
3 AS5600 magnetic rotary encoders;  
PD / QC PPS / QC4+ Type-C USB trigger pad for power supply;
TCA9548A / PCA9548A multiplexer;
Arduino Uno;
laptop for controlling manipulator.

All other elements but screws are 3-d printed with PLA.

for now is done code for controlling any stepper by encoder, am waiting for delivering TCA9548A / PCA9548A multiplexer to control several AS5600 together. Code allows to control sg90 grabber but arduino cannot produce enough power without extra power supply to stay connected to laptop.
