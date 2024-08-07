tiny robo-arm project.

Currently is in progress. 

I am making 3-axis robo-arm with these modules:
two Nema 17 23 mm stepping motors;
Nema 17 40 mm stepping motor;
SG90 servo motor;
three A4988 motor encoders;
3 AS5600 magnetic rotary encoders;  
PD / QC PPS / QC4+ Type-C USB trigger pad for power supply;
TCA9548A / PCA9548A multiplexer;
Mini360 voltage convertor for servo;
Arduino Uno;
laptop for controlling manipulator.



Libraries:

https://github.com/RobTillaart/AS5600

https://www.arduino.cc/reference/en/libraries/servo/

https://www.arduino.cc/reference/en/libraries/accelstepper/



All other elements but screws are 3-d printed with PLA.

for now is done code for controlling any stepper by encoder, am waiting for delivering TCA9548A / PCA9548A multiplexer to control several AS5600 together.

Steppers posiitons are controlled by 3 coordinates: angle of the first axis, height and radius of grabber. Functions for calculating all 3 angles are ready for tests.
