#include <AccelStepper.h>
#include "AS5600.h"
#include <Servo.h>
#include <math.h>

#define ARM_PIN 0

//mm
const int element_length[] = {0, 62, 69, 115}; //from the model
const int element_height[] = {69, -29, 0, 0};

const int default_fi = 180;
const int default_dist = 200;
const int default_height = 0;

const char default_string[] = "---------";
const char endl = 'e';
const char pause = 'p';
const char play = 'c';
const char grab = 'g';
const char default_pos = 'd';

const float angle_dislocation[] = {-22.94, 0, 0}; //dir pin connect to 5v
const byte delta = 1;

String string = default_string;

int step_delay = 15;

bool is_grabbed = 1;
char input = '0';
long target_pos_uncut = 0;

int target_fi = default_fi;
int target_dist = default_dist;
int target_height = default_height;

byte i = 0;

float angle_1 = 0.0;
float angle_2 = 0.0;
float angle_3 = 0.0;

int target_pos_1 = int (target_fi / 1.8);
int target_pos_2 = 100;
int target_pos_3 = 100;

AccelStepper Stepper1(1,3,2);
AS5600 encoder1;  
Servo Arm;


void arm_setup(Servo my_servo){
  my_servo.attach(ARM_PIN);
  my_servo.write(90);
}

void arm_lock(Servo my_servo){
  int position = my_servo.read();
  if (position==0){
    return;
  }
  my_servo.write(0);
}

void arm_unlock(Servo my_servo){
  int position = my_servo.read();
  if (position==150){
    return;
  }
  my_servo.write(150);
}

void arm_off(Servo my_servo){
  arm_lock(my_servo);
  my_servo.detach();
}

void arm_grab_release(Servo my_servo){
  if (is_grabbed){
    Serial.println("releasing");
    arm_unlock(my_servo);
    is_grabbed = 0;
  }
  else {
    Serial.println("grabbing");
    arm_lock(my_servo);
    is_grabbed = 1;
  }
  delay(1000);//чтобы успеть прочесть текст
}

void waiting(){
  i = 0;
  string = default_string;
  Serial.println("PRESS <C> TO CONTINUE");
  while(Serial.read() != play){
    delay(100);
  }
  Serial.println("THE PROGRAM WILL CONTINUE IN 1 SECOND");
  delay(1000);
}

void set_default_pos(){
  target_fi = default_fi;
  target_dist = default_dist;
  target_height = default_height;
}

void add_char(char input_char){
      switch (input_char)
    {
      case endl:
        break;

      case default_pos:
        set_default_pos();
        break;

      case grab:
        arm_grab_release(Arm);
        break;
      
      case pause:
        waiting();
        break;
      
      default:
        string[i] = input_char;
    }
}

void check_input(){
  if (Serial.available() <= 0){
    return;
  }
  input = Serial.read();
  add_char(input);

  if ((i < 8) and (input != endl)){
    i++;
  }
}

void get_coords(){
  if (i!=8){
    return;
  }
  target_pos_uncut = string.toInt();
  int fi = target_pos_uncut / 1000 / 1000;
  int dist = target_pos_uncut / 1000 % 1000;
  int height = target_pos_uncut % 1000 % 1000;

  if ((fi <= 0) or (fi >= 330)){
    return;
  }
  target_fi = fi;

  if ((dist <= -70) or (dist > (element_length[0] + element_length[1] + element_length[2] + element_length[3]))){
    return;
  }
  if ((height < 5) or (height > (element_height[0] + element_height[1] + element_height[2] + element_height[3]))){
    return;
  }

  target_dist = dist;
  target_height = height;
}

void read_input(){
  if ((string == default_string) or (input != endl)){
    return;
  }

  get_coords();
  string = default_string;
  i = 0;
}

void get_target_pos_1(){
  if ((target_fi <= 0) or (target_fi >= 330)){
    return;
  }
  target_pos_1 = target_fi / 1.8;
}

void get_target_pos_2(){
  double q3 = 0.0;
  double cos_q3 = square(target_dist - element_length[0] - element_length[1]);
  cos_q3 += square(target_height - element_height[0] - element_height[1]);
  cos_q3 -= square(element_length[2]) + square(element_length[3]);
  cos_q3 /= 2 * element_length[2] * element_length[3];

  q3 = -1 * acos(cos_q3) * 180 / M_PI;
  //тут *(-1) если в диапазоне где надо развернуть
  if ((q3 >= 320) or (q3 <= 170)){
    return;
  }

  target_pos_3 = int(q3 / 1.8);
}

void get_target_pos_3(){
  double q2 = 0.0;
  double q3 = double(target_pos_3) * M_PI / 100;

  double tg_2 = element_length[3] * sin(q3);
  double tg_1 = target_height - element_height[0] - element_height[1];
  tg_1 /= target_dist - element_length[0] - element_length[1];
  tg_2 /= element_length[3] * sin(q3) + element_length[2];

  q2 = atan(tg_1) - atan(tg_2);
  q2 *= 180 / M_PI;
  if ((q2 >= 320) or (q2 <= 60)){
    return;
  }
  target_pos_2 = int(q2 / 1.8);
}

void print_target_coords(){
  //Serial.println(string);
  
  Serial.print(target_pos_uncut);
  Serial.print("\t");

  Serial.print(target_fi);
  Serial.print("\t");
  Serial.print(target_dist);
  Serial.print("\t");
  Serial.println(target_height);
}

void encoder_setup(AS5600 enc){

  Wire.begin();

  enc.begin();
  Serial.print("Connect: ");
  Serial.println(enc.isConnected());
  delay(100);
}

float angle(AS5600 enc, float angle_dislocation){
  if (!enc.isConnected()){
    return default_fi;
  }
  return (float(enc.rawAngle()) / 4096 * 360) - angle_dislocation;
}

int current_position(float current_angle){
  return int((current_angle)/1.8);
}

void stepper_setup(AccelStepper Stepper, float current_angle){
  Stepper.setMaxSpeed(100);
  Stepper.setAcceleration(50);
  Stepper.setCurrentPosition(current_position(current_angle));
}

void stepper_print(AccelStepper Stepper, float angle){

  Serial.print(angle);
  Serial.print("\t ");
  Serial.print(current_position(angle));
  //Serial.print("\t");
  //Serial.print(Stepper.currentPosition());
  Serial.print("\n");
}

//продумать для нескольких двигателей
//можно находить какому двигателю надо проехать дальше и выбирать его для отсчета
void speed_regulation(int target_position, float current_angle){
  float k_p = 200.0;
  float div = 1 / float(abs(current_position(current_angle) - target_position) + 5);
  step_delay = int(div * k_p) + 5;

}

void fix_position(int target_position, float current_angle, AccelStepper Stepper){
  
  if (abs(current_position(current_angle) - target_position) <= delta){
    Stepper.setCurrentPosition(target_position);
  }
  if (current_position(current_angle) - target_position > delta){
    Stepper.move(-1);
  }
  if (current_position(current_angle) - target_position < -delta){
    Stepper.move(1);
  }

  Stepper.run();
}


void setup() {
  Serial.begin(115200);

  arm_setup(Arm);
  encoder_setup(encoder1);
  stepper_setup(Stepper1, angle(encoder1, angle_dislocation[0]));

}


void loop() {
  angle_1 = angle(encoder1, angle_dislocation[0]);

  check_input();
  read_input();
  get_target_pos_1();
  
  fix_position(target_pos_1, angle_1, Stepper1);
  
  speed_regulation(target_pos_1, angle_1);
  
  stepper_print(Stepper1, angle_1);
  print_target_coords();

  delay(step_delay);
}
