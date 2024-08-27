#include <AccelStepper.h>
#include <Wire.h>
#include "AS5600.h"
#include "TI_TCA9548A.h"
#include <Servo.h>
#include <math.h>

#define ARM_PIN 1
const byte enc_adress[] = {5, 6, 7};

const char default_string[] = "---------";
const char endl = 'e';
const char pause = 'p';
const char play = 'c';
const char grab = 'g';
const char angles = 'a';
const char default_pos = 'd';

//mm //заменить на новые размеры
const int element_length[] = {0, 70, 70, 114}; //from the model
const int element_height[] = {70, -29, 0, 0};

const int default_fi = 180;
const int default_dist = 230;
const int default_height = 50;

const float angle_dislocation[] = {-22.94, 6.42 + 180, -5.27 + 180};
const float reduction[] = {1.0, 4.0, 1.0};
const byte delta = 1;

const bool clockwise_direction[] = {0, 1, 0};

String string = default_string;

int step_delay = 15;

bool is_grabbed = 1;
char input = '0';

int target_fi = default_fi;
int target_dist = default_dist;
int target_height = default_height;

byte i = 0;

float enc_angle[] = {0.0, 0.0, 0.0};
int target_pos[] = {int (target_fi / 1.8), 0, 0}; //цилиндрические координаты

AccelStepper Stepper0(1,9,8);
AccelStepper Stepper1(1,6,5);
AccelStepper Stepper2(1,3,2);
AccelStepper element_steppers[] = {Stepper0, Stepper1, Stepper2};

//AS5600 encoder0;  //dir pin connected to 5v
//AS5600 encoder1;  //dir pin connected to gnd
//AS5600 encoder2;  //dir pin connected to 5v
AS5600 element_encoders[3]; //= {encoder0, encoder1, encoder2};

Servo Arm;


void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

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

      case angles:
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

  if ((i < 8) and ((input != endl) or (input != angles))){
    i++;
  }
}

void get_angles(){
  if (i!=8){
    return;
  }
  String string_q0 = string;
  String string_q1 = string;
  String string_q2 = string;
  string_q0.remove(3);
  string_q1.remove(0, 3);
  string_q1.remove(3);
  string_q2.remove(0, 6);

  int angle_0 = string_q0.toInt();
  int angle_1 = string_q1.toInt();
  int angle_2 = string_q2.toInt();

  if ((angle_0 <= 30) or (angle_0 >= 330)){
    return;
  }
  if ((angle_1 >= 120) or (angle_1 <= -20)){
    return;
  }
  if ((angle_2 >= 120) or (angle_2 <= -120)){
    return;
  }

  target_pos[0] = int(double(angle_0) / 1.8 * reduction[0]);
  target_pos[1] = int(double(angle_1) / 1.8 * reduction[1]);
  target_pos[2] = int(double(angle_2) / 1.8 * reduction[2]);

}

void get_coords(){
  if (i!=8){
    return;
  }

  String string_fi = string;
  String string_height = string;
  String string_dist = string;
  string_fi.remove(3);
  string_dist.remove(0, 3);
  string_dist.remove(3);
  string_height.remove(0, 6);

  int fi = string_fi.toInt();
  int dist = string_dist.toInt();
  int height = string_height.toInt();

  if ((fi <= 30) or (fi >= 330)){
    return;
  }
  if ((dist <= -70) or 
  (dist > (element_length[0] + element_length[1] + element_length[2] + element_length[3]))){
    return;
  }
  if ((height < 20) or 
  (height > (element_height[0] + element_height[1] + element_length[2] + element_length[3]))){
    return;
  }
  //теорема пифагора
  if ((square(dist - element_length[0] - element_length[1]) + 
  square(height - element_height[0] - element_height[1])) > 
  square(element_length[2] + element_length[3])){
    return;
  }
  
  //отрезает всю зону 1 и 2 звена
  if ((dist < 100) and (height < 75)){
    return;
  }

  target_fi = fi;
  target_dist = dist;
  target_height = height;
}

void get_target_pos_0(){
  target_pos[0] = int(target_fi / 1.8 * reduction[0]);
}

void get_target_pos_1_2(){
  double q2 = 0.0;
  double cos_q3 = square(target_dist - element_length[0] - element_length[1]);
  cos_q3 += square(target_height - element_height[0] - element_height[1]);
  cos_q3 -= square(element_length[2]) + square(element_length[3]);
  cos_q3 /= 2 * element_length[2] * element_length[3];

  if (abs(cos_q3) > 1){
    return;
  }

  q2 = -1 * acos(cos_q3) * 180 / M_PI;
  
  //если заходит в обратное направление наклона - считать угол в другую сторону
  if (target_dist <= element_length[0] + element_length[1]){
    q2 *= -1;
  }
  
  if ((q2 >= 120) or (q2 <= -120)){
    return;
  }

  double q1 = 0.0;
  double tg_2 = element_length[3] * sin(q2);
  double tg_1 = target_height - element_height[0] - element_height[1];
  tg_1 /= target_dist - element_length[0] - element_length[1];
  tg_2 /= element_length[3] * cos(q2) + element_length[2];

  q1 = atan(tg_1) - atan(tg_2);
  q1 *= -1 * 180 / M_PI;
  if ((q1 >= 20) or (q1 <= -120)){
    return;
  }
  target_pos[1] = int(q1 / 1.8 * reduction[1]);
  target_pos[2] = int(q2 / 1.8 * reduction[2]);

}

//отключена обратная кинематика
void read_input(){
  if (string == default_string){
    return;
  }

  switch (input){
    case endl:
      get_coords();
      get_target_pos_0();
      //get_target_pos_1_2();
      break;

    case angles:
      get_angles();
      break;

    default:
      return;
  }

  string = default_string;
  i = 0;
}

void print_target_coords(){
  Serial.print("\n");
  //Serial.println(string);

  Serial.print(target_fi);
  Serial.print("\t");
  Serial.print(target_dist);
  Serial.print("\t");
  Serial.println(target_height);
}

void print_target_positions(){
  Serial.print("\n");
  //Serial.println(string);

  Serial.print(target_pos[0]);
  Serial.print("\t");
  Serial.print(target_pos[1]);
  Serial.print("\t");
  Serial.println(target_pos[2]);
}


void encoder_setup(AS5600 enc){
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
  //Serial.print("\t");
  //Serial.print(Stepper.currentPosition());
  Serial.print("\t");
}

//продумать для нескольких двигателей
//можно находить какому двигателю надо ехать дальше/ближе и выбирать его для отсчета
void speed_regulation(int target_position, float current_angle, float reduct){
  float k_p = 200.0;
  float div = 1 / float(abs(current_position(current_angle, reduct) - target_position) + 5);
  step_delay = int(div * k_p) + 5;

}

void fix_position(int target_position, float current_angle, AccelStepper Stepper, bool direction, float reduct){
  int step = 1;
  int pos = current_position(current_angle, reduct);
  if (direction){
    step *= (-1);
  }

  if (abs(pos - target_position) <= delta){
    Stepper.setCurrentPosition(target_position);
    return;
  }
  if (pos - target_position > delta){
    Stepper.move(-step);
  }
  if (pos - target_position < -delta){
    Stepper.move(step);
  }

  Stepper.run();
}


void servo_setup(int index){
  if ((index < 0) or (index > 2)){
    return;
  }

  TCA9548A(enc_adress[index]);
  encoder_setup(element_encoders[index]);
  stepper_setup(element_steppers[index], angle(element_encoders[index], angle_dislocation[index]), reduction[index]);
}

float servo_angle(int index){
  if ((index < 0) or (index > 2)){
    return;
  }

  TCA9548A(enc_adress[index]);
  return angle(element_encoders[index], angle_dislocation[index]);
}

void fix_servo_position(int index){
  if ((index < 0) or (index > 2)){
    return;
  }

  fix_position(target_pos[index], enc_angle[index], element_steppers[index], clockwise_direction[index], reduction[index]);
}

void print_servo_position(int index){
  if ((index < 0) or (index > 2)){
    return;
  }

  stepper_print(element_steppers[index], enc_angle[index], reduction[index]);
}


void setup() {
  Serial.begin(115200);
  Wire.begin();

  arm_setup(Arm);

  servo_setup(0);
  servo_setup(1);
  servo_setup(2);

  delay(2000);
}


void loop() {

  enc_angle[0] = servo_angle(0);
  enc_angle[1] = servo_angle(1);
  enc_angle[2] = servo_angle(2);

  check_input();
  read_input();
  
  fix_servo_position(0);
  fix_servo_position(1);
  fix_servo_position(2);
  
  print_servo_position(0);
  print_servo_position(1);
  print_servo_position(2);
  //print_target_coords();
  print_target_positions();

  speed_regulation(target_pos[0], enc_angle[0], reduction[0]);
  delay(step_delay);
}
