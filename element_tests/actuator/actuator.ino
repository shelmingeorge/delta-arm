#include <AccelStepper.h>
#include <Wire.h>
#include "AS5600.h"
#include "TI_TCA9548A.h"

const int enc_adress = 6;
const float angle_dislocation = 31.36 + 180.0;
const byte delta = 10;
const bool clockwise_direction = 1;

int step_delay = 15;

float reduction = 4.0;
float angle_1 = 0.0;

float target_angle = 90.0;
int target_pos = int(target_angle / 1.8 * reduction);

AccelStepper Stepper1(1,3,2);
AS5600 enc1;

void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

void encoder_setup(AS5600 enc){
  enc.begin();
  Serial.print("Connect: ");
  Serial.println(enc.isConnected());
  delay(100);
}

float angle(AS5600 enc, float angle_dislocation){

  return (float(enc.rawAngle()) / 4096 * 360) - angle_dislocation;
}

int current_position(float current_angle, float reduct){
  return int((current_angle) / 1.8 * reduct);
}

void stepper_setup(AccelStepper Stepper, float current_angle, float reduct){
  Stepper.setMaxSpeed(100);
  Stepper.setAcceleration(50);
  Stepper.setCurrentPosition(current_position(current_angle, reduct));
}

void stepper_print(AccelStepper Stepper, float angle, float reduct){
  Serial.print(angle);
  Serial.print("\t ");
  Serial.print(current_position(angle, reduct));
  Serial.print("\t");
}

void fix_position(int target_position, float current_angle, AccelStepper Stepper, bool direction, float reduct){
  int step = 1;
  int pos = current_position(current_angle, reduct);

  if (abs(pos - target_position) <= delta){
    //Stepper.setCurrentPosition(target_position);
    return;
  }

  if (direction){
    step *= (-1);
  }

  if (pos - target_position > delta){
    Stepper.move(-step);
  }
  if (pos - target_position < -delta){
    Stepper.move(step);
  }

  Stepper.run();
}


void setup() {
  Serial.begin(115200);
  Wire.begin();

  TCA9548A(enc_adress);
  encoder_setup(enc1);
  stepper_setup(Stepper1, angle(enc1, angle_dislocation), reduction);

  delay(2000);
}

void loop() {
  angle_1 = angle(enc1, angle_dislocation);
  fix_position(target_pos, angle_1, Stepper1, clockwise_direction, reduction);

  stepper_print(Stepper1, angle_1, reduction);
  Serial.println("\n");
  delay(step_delay);
}
