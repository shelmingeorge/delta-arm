#include <AccelStepper.h>
#include "AS5600.h"

const byte step_delay = 40;
const float angle_1_dislocation = 22.94;
const byte delta = 1;
float angle_1 = 0.0;

float target_angle = 180.0;
int target_pos_1 = int (target_angle / 1.8);

AccelStepper Stepper1(1,3,2);
AS5600 encoder1;  

void encoder_setup(AS5600 enc){

  Wire.begin();

  enc.begin();
  Serial.print("Connect: ");
  Serial.println(enc.isConnected());
  delay(100);
}

float angle(AS5600 enc, float angle_dislocation){
  if (!enc.isConnected()){
    return 180.0;
  }
  return (float(enc.rawAngle()) / 4096 * 360) - angle_dislocation;
}

int current_position(float current_angle){
  return int((current_angle)/1.8);
}

void stepper_setup(AccelStepper Stepper, float current_angle){
  Stepper.setMaxSpeed(200);
  Stepper.setAcceleration(200);
  Stepper.setCurrentPosition(current_position(current_angle));
}

void stepper_print(AccelStepper Stepper, float angle){

  Serial.print(angle);
  Serial.print("\t ");
  Serial.print(current_position(angle));
  //Serial.print("\t");
  //Serial.println(Stepper.currentPosition());
}

void fix_position(int target_position, float current_angle, AccelStepper Stepper){
  
  if (abs(current_position(current_angle) - target_position) <= delta){
    Stepper.setCurrentPosition(target_position);
  }
  if (current_position(current_angle) - target_position > delta){
    Stepper.move(1);
  }
  if (current_position(current_angle) - target_position < -delta){
    Stepper.move(-1);
  }

  Stepper.run();
}


void setup() {
  Serial.begin(9600);

  encoder_setup(encoder1);
  stepper_setup(Stepper1, angle(encoder1, angle_1_dislocation));

}


void loop() {

  angle_1 = angle(encoder1, angle_1_dislocation);
  fix_position(target_pos_1, angle_1, Stepper1);
  stepper_print(Stepper1, angle_1);

  delay(step_delay);
}
