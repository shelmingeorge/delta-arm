#include <Servo.h>

const int ARM_PIN = 10;
Servo Arm;

void Arm_prep(Servo my_servo){
  my_servo.attach(ARM_PIN);
  my_servo.write(90);
}

void Arm_lock(Servo my_servo){
  int position = my_servo.read();
  if (position==0){
    return;
  }
  my_servo.write(0);
}

void Arm_unlock(Servo my_servo){
  int position = my_servo.read();
  if (position==180){
    return;
  }
  my_servo.write(180);
}

void Arm_off(Servo my_servo){
  Arm_lock(my_servo);
  my_servo.detach();
}

void setup() {
  Arm_prep(Arm);

}

void loop() {
  Arm_unlock(Arm);
  delay(2000);
  Arm_lock(Arm);
  delay(2000);

  Arm_off(Arm);
}
