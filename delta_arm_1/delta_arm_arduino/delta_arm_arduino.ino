#include <AccelStepper.h>
#include <Wire.h>
#include "AS5600.h"
#include "TI_TCA9548A.h"
#include <Servo.h>
#include <math.h>

#define ARM_PIN 11
const byte ENC_ADRESS[] = {5, 6, 7};

//mm //updated
const int ELEMENT_LENGTH[] = {0, 92, 70, 118}; //from the model
const int ELEMENT_HEIGHT[] = {73, -30, 0, 0};

const float ANGLE_DISLOCATION[] = {-13.94, 1.42 + 180, -5.27 + 180};
const float REDUCTION[] = {1.0, 4.0, 1.0};
const bool SPIN_DIRECTION[] = {1, 1, 0};
//0 - counterclockwize, 1 - down, 2 - up
const char DEFAULT_STRING[] = "---------";
String string = DEFAULT_STRING;

enum Control_chars : char {endl = 'e', angles = 'a',
  pause = 'p', play = 'c', grab = 'g', default_pos = ' ', write_pos = 'w',
  up = 'u', down = 'd', left = 'l', right = 'r', forward = 'f', backward = 'b'};

const byte DELTA = 1;
const byte MOVE_STEPS_PER_COMMAND = 3;
const byte MOVE_MM_PER_COMMAND = 5;
const float MOVE_DEGREES_PER_COMMAND = REDUCTION[0] * 1.8 * MOVE_STEPS_PER_COMMAND;

char input = '0';

int target_fi = 40;//180; // 40
int target_dist = 185;//ELEMENT_LENGTH[0] + ELEMENT_LENGTH[1] + ELEMENT_LENGTH[2] + ELEMENT_LENGTH[3]; // 185
int target_height = 80;//ELEMENT_HEIGHT[0] + ELEMENT_HEIGHT[1] + ELEMENT_HEIGHT[2] + ELEMENT_HEIGHT[3]; // 80

byte i = 0;
bool is_grabbed = 1;
bool are_enconers_connected = true; //do not move if any encoder is disconnected
bool ready_for_pc_command = false;

float enc_angle[] = {0.0, 0.0, 0.0};
const int DEFAULT_POSITIONS[] = {22, -239, -60};//{100, 0, 0}; // {22, -239, -60};
int target_pos[] = {DEFAULT_POSITIONS[0], DEFAULT_POSITIONS[1], DEFAULT_POSITIONS[2]}; // cylindric coords
int prev_pos[] = {0, 0, 0}; // for pc_check

AccelStepper Stepper0(1,9,8);
AccelStepper Stepper1(1,6,5);
AccelStepper Stepper2(1,3,2);
AccelStepper element_steppers[] = {Stepper0, Stepper1, Stepper2};

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
  if (position <= 40){
    return;
    }
  my_servo.write(40);
  }

void arm_unlock(Servo my_servo){
  int position = my_servo.read();
  if (position >= 150){
    return;
    }
  my_servo.write(150);
  }

void arm_off(Servo my_servo){
  //arm_lock(my_servo);
  my_servo.detach();
  }

void arm_grab_release(Servo my_servo){
  if (is_grabbed){
    //Serial.println("releasing");
    arm_unlock(my_servo);
    is_grabbed = 0;
    }
  else {
    //Serial.println("grabbing");
    arm_lock(my_servo);
    is_grabbed = 1;
    }
  delay(1000);//чтобы успеть прочесть текст
  }

void waiting(){
  i = 0;
  string = DEFAULT_STRING;
  Serial.println("PRESS <C> TO CONTINUE");
  while(Serial.read() != play){
    delay(100);
    }
  Serial.println("THE PROGRAM WILL CONTINUE IN 1 SECOND");
  delay(1000);
  }

void set_default_pos(){
  target_pos[0] = DEFAULT_POSITIONS[0];
  target_pos[1] = DEFAULT_POSITIONS[1];
  target_pos[2] = DEFAULT_POSITIONS[2];
  
  target_fi = 40;//180; // 40
  target_dist = 185;//ELEMENT_LENGTH[0] + ELEMENT_LENGTH[1] + ELEMENT_LENGTH[2] + ELEMENT_LENGTH[3]; // 185
  target_height = 80;//ELEMENT_HEIGHT[0] + ELEMENT_HEIGHT[1] + ELEMENT_HEIGHT[2] + ELEMENT_HEIGHT[3]; // 80
  }

bool check_collisions(int target_pos_0, int target_pos_1, int target_pos_2){
  return true;
  double angle_0 = float(target_pos_0) * 1.8 / REDUCTION[1];
  if ((angle_0 < 30) and (angle_0 > 330)){
    return false;
    }
  //
  enum movement_limits : int {dist_min_limit = 0, dist_max_limit = 280, bottom_limit = 0, top_limit = 220};
  enum hurtbox1_sizes : int {h1_lenght = 80, h1_height = 40, h1_bottom = -5, h1_left = 40};
  const int hitbox_coords[] = {120, 73}; // dist and height from (0,0)

  float target_angles[] = {
    target_pos_1 * 1.8 / REDUCTION[1], 
    target_pos_2 * 1.8 / REDUCTION[2]};
  if (SPIN_DIRECTION[1]) target_angles[0] *= (-1);

  target_angles[0] *= M_PI / 180.0;
  target_angles[1] *= M_PI / 180.0;

  int hurtbox0_Xc_Yc_R[] = {
    ELEMENT_LENGTH[0] + ELEMENT_LENGTH[1] + ELEMENT_LENGTH[2] * cos(target_angles[0]),
    ELEMENT_HEIGHT[0] + ELEMENT_HEIGHT[1] + ELEMENT_LENGTH[2] * sin(target_angles[0]),
    20};

  if ((hurtbox0_Xc_Yc_R[0] - hurtbox0_Xc_Yc_R[2]) < dist_min_limit) {
    //Serial.println("h0 min dist limit");
    //delay(800);
    return false;
    }/*
  if ((hurtbox0_Xc_Yc_R[0] + hurtbox0_Xc_Yc_R[2]) > dist_max_limit) {
    Serial.println("h0 max dist limit");
    delay(1000);
    return false;
    }*/
  if ((hurtbox0_Xc_Yc_R[1] - hurtbox0_Xc_Yc_R[2]) < bottom_limit) {
    //Serial.println("h0 bottom limit");
    //delay(800);
    return false;
    }/*
  if ((hurtbox0_Xc_Yc_R[1] + hurtbox0_Xc_Yc_R[2]) > top_limit) {
    Serial.println("h0 top limit");
    delay(1000);
    return false;
    }*/

  if (((hurtbox0_Xc_Yc_R[0] - hurtbox0_Xc_Yc_R[2]) < hitbox_coords[0]) and 
    ((hurtbox0_Xc_Yc_R[1] - hurtbox0_Xc_Yc_R[2]) < hitbox_coords[1])){
      //Serial.println("h0 hitbox limit");
      //delay(800);
      return false;
      }

  //  top near point
    int hurtbox1_coords[][2] = {//  {X, Y}
      {hurtbox0_Xc_Yc_R[0] + int((h1_height + h1_bottom) * cos(target_angles[1] + M_PI / 2)) + 
        int(h1_left * cos(target_angles[1])),
      hurtbox0_Xc_Yc_R[1] + int((h1_height + h1_bottom) * sin(target_angles[1] + M_PI / 2)) + 
        int(h1_left * sin(target_angles[1])) },
      {0, 0}, {0, 0}};
  //  top distant point
    hurtbox1_coords[1][0] = hurtbox1_coords[0][0] + int(h1_lenght  *cos(target_angles[1]));
    hurtbox1_coords[1][1] = hurtbox1_coords[0][1] + int(h1_lenght * sin(target_angles[1]));
  //  bottom distant point
    hurtbox1_coords[2][0] = hurtbox1_coords[1][0] + int((h1_height + h1_bottom) * cos(target_angles[1] - M_PI / 2));
    hurtbox1_coords[2][1] = hurtbox1_coords[1][1] + int((h1_height + h1_bottom) * sin(target_angles[1] - M_PI / 2));  
  
  for (byte i = 0; i < 3; i++){
    //Serial.print(hurtbox1_coords[i][0]);
    //Serial.print('\t');
    //Serial.println(hurtbox1_coords[i][1]);
    //delay(800);
    if (hurtbox1_coords[i][0] < dist_min_limit) {
      //Serial.println("h1 min dist limit");
      //delay(800);
      return false;
      }/*
    if (hurtbox1_coords[i][0] > dist_max_limit) {
      Serial.println("h1 max dist limit");
      delay(1000);
      return false;
      }*/
    if (hurtbox1_coords[i][1] < bottom_limit) {
      //Serial.println("h1 bottom limit");
      //delay(800);
      return false;
      }/*
    if (hurtbox1_coords[i][1] > top_limit) {
      Serial.println("h1 top limit");
      delay(1000);
      return false;
      }*/

    if ((hurtbox1_coords[i][0] < hitbox_coords[0]) and (hurtbox1_coords[i][1] < hitbox_coords[1])){
      //Serial.println("h1 hitbox limit");
      //delay(800);
      return false;
      }
    }
  return true;
  }

void move_up(){
  int old_height = target_height;
  int old_pos[] = {target_pos[1], target_pos[2]};

  target_height += MOVE_MM_PER_COMMAND;
  get_target_positions();

  if ((target_pos[1] == old_pos[0]) and (target_pos[2] == old_pos[1])){
    target_height = old_height;
    }
  }

void move_down(){
  int old_height = target_height;
  int old_pos[] = {target_pos[1], target_pos[2]};

  target_height -= MOVE_MM_PER_COMMAND;
  get_target_positions();

  if ((target_pos[1] == old_pos[0]) and (target_pos[2] == old_pos[1])){
    target_height = old_height;
    }
  }

void move_left(){
  if (target_fi + MOVE_DEGREES_PER_COMMAND >= 330){
      return;
    }
  target_fi += MOVE_DEGREES_PER_COMMAND;
  target_pos[0] += MOVE_STEPS_PER_COMMAND;
  }

void move_right(){
  if (target_fi - MOVE_DEGREES_PER_COMMAND <= 30){
      return;
    }
  target_fi -= MOVE_DEGREES_PER_COMMAND;
  target_pos[0] -= MOVE_STEPS_PER_COMMAND;
  }

void move_forward(){
  int old_dist = target_dist;
  int old_pos[] = {target_pos[1], target_pos[2]};

  target_dist += MOVE_MM_PER_COMMAND;
  get_target_positions();

  if ((target_pos[1] == old_pos[0]) and (target_pos[2] == old_pos[1])){
    target_dist = old_dist;
    }
  }

void move_backward(){
  int old_dist = target_dist;
  int old_pos[] = {target_pos[1], target_pos[2]};

  target_dist -= MOVE_MM_PER_COMMAND;
  get_target_positions();

  if ((target_pos[1] == old_pos[0]) and (target_pos[2] == old_pos[1])){
    target_dist = old_dist;
    }
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
      
      case up:
        move_up();
        break;
      
      case down:
        move_down();
        break;   
      
      case left:
        move_left();
        break;
      
      case right:
        move_right();
        break;

      case write_pos:
        print_all_info();
        break;

      case forward:
        move_forward();
        break;

      case backward:
        move_backward();
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
  int angle_1 = -1 * string_q1.toInt();
  int angle_2 = string_q2.toInt();

  if (!check_collisions(
    int(double(angle_0) / 1.8 * REDUCTION[0]),
    int(double(angle_1) / 1.8 * REDUCTION[1]),
    int(double(angle_2) / 1.8 * REDUCTION[2]))){
    return;
    }

  target_pos[0] = int(double(angle_0) / 1.8 * REDUCTION[0]);
  target_pos[1] = int(double(angle_1) / 1.8 * REDUCTION[1]);
  target_pos[2] = int(double(angle_2) / 1.8 * REDUCTION[2]);
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

  target_fi = fi;
  target_dist = dist;
  target_height = height;
  }


void get_target_positions(){
  double q2 = target_pos[2];
  
  double acc = target_dist - ELEMENT_LENGTH[0] - ELEMENT_LENGTH[1];
  double cos_q2 = sq(acc);
  acc = target_height - ELEMENT_HEIGHT[0] - ELEMENT_HEIGHT[1];
  cos_q2 += sq(acc);
  cos_q2 -= sq(ELEMENT_LENGTH[2]) + sq(ELEMENT_LENGTH[3]);
  cos_q2 /= (2 * ELEMENT_LENGTH[2] * ELEMENT_LENGTH[3]);

  if (abs(cos_q2) > 1){
    return;
    }

  q2 = acos(cos_q2);

  //если заходит в обратное направление наклона - считать угол в другую сторону
  if (target_dist > (ELEMENT_LENGTH[0] + ELEMENT_LENGTH[1])) q2 *= -1;

  double q1 = target_pos[1];
  double tg_2 = ELEMENT_LENGTH[3] * sin(q2);
  double tg_1 = target_height - ELEMENT_HEIGHT[0] - ELEMENT_HEIGHT[1];
  tg_1 /= target_dist - ELEMENT_LENGTH[0] - ELEMENT_LENGTH[1];
  tg_2 /= ELEMENT_LENGTH[3] * cos(q2) + ELEMENT_LENGTH[2];

  q1 = atan(tg_1) - atan(tg_2);
  q1 *= 180.0 / M_PI; 

  //если заходит в обратное направление наклона - считать угол в другую сторону
  if (target_dist <= ELEMENT_LENGTH[0] + ELEMENT_LENGTH[1]) q1 = 180.0 + q1;

  if (SPIN_DIRECTION[1]) q1 *= -1; //направление отсчета двигателя
  
  q2 *= 180.0 / M_PI;
  
  if (!check_collisions(
    int(target_fi / 1.8 * REDUCTION[0]),
    int(q1 / 1.8 * REDUCTION[1]),
    int(q2 / 1.8 * REDUCTION[2]))){
    return;
    }
  
  target_pos[0] = int(target_fi / 1.8 * REDUCTION[0]);
  target_pos[1] = int(q1 / 1.8 * REDUCTION[1]);
  target_pos[2] = int(q2 / 1.8 * REDUCTION[2]);

  }

void read_input(){
  if (string == DEFAULT_STRING){
    return;
    }

  switch (input){
    case endl:
      get_coords();
      get_target_positions();
      break;

    case angles:
      get_angles();
      break;

    default:
      return;
    }

  string = DEFAULT_STRING;
  i = 0;
  }

void print_target_coords(){
  Serial.print("\n");

  Serial.print(target_fi);
  Serial.print("\t");
  Serial.print(target_dist);
  Serial.print("\t");
  Serial.println(target_height);
  }

void print_target_positions(){
  Serial.print("\n");

  Serial.print(target_pos[0]);
  Serial.print("\t");
  Serial.print(target_pos[1]);
  Serial.print("\t");
  Serial.println(target_pos[2]);
  }

void encoder_setup(AS5600 enc){
  enc.begin();
  //Serial.print("Connect: ");
  //Serial.println(enc.isConnected());
  if(!enc.isConnected()){
    are_enconers_connected = false;
    }
  delay(100);
  }

float angle(AS5600 enc, float ANGLE_DISLOCATION){
  if (!enc.isConnected()){
    return 400;
    }
  return (float(enc.rawAngle()) / 4096 * 360) - ANGLE_DISLOCATION;
  }

int current_position(float current_angle, float reduct){
  return int((current_angle) / 1.8 * reduct);
  }

void stepper_setup(AccelStepper Stepper, float current_angle, float reduct){
  Stepper.setMaxSpeed(int (50 * reduct));
  Stepper.setAcceleration(int(50 * reduct));
  Stepper.setCurrentPosition(current_position(current_angle, reduct));
  }

void stepper_print(AccelStepper Stepper, float angle, float reduct){
  Serial.print(angle);
  Serial.print("\t ");
  Serial.print(current_position(angle, reduct));
  Serial.print("\t");
  }

void speed_regulation(int target_position, float current_angle, float reduct){
  int step_delay = 20;

  float k_p = 200.0;
  float div = 1 / float(abs(current_position(current_angle, reduct) - target_position) + 5);
  step_delay = int(div * k_p) + 5;

  delay(step_delay);
  }

void fix_position(int target_position, float current_angle, AccelStepper Stepper, bool direction, float reduct){
  if (current_angle > 359){
    return;
    }
 
  int step = 1;
  int pos = current_position(current_angle, reduct);
  
  if (abs(pos - target_position) <= DELTA){
    Stepper.setCurrentPosition(target_position);
    return;
    }
  if (direction){
    step *= (-1);
    }
  
  if (pos - target_position > DELTA){
    Stepper.move(-step);
    }
  if (pos - target_position < -DELTA){
    Stepper.move(step);
    }

  Stepper.run();
  }

void servo_setup(int index){
  if ((index < 0) or (index > 2)){
    return;
    }

  TCA9548A(ENC_ADRESS[index]);
  encoder_setup(element_encoders[index]);
  stepper_setup(element_steppers[index], angle(element_encoders[index], ANGLE_DISLOCATION[index]), REDUCTION[index]);
  }

float servo_angle(int index){
  if ((index < 0) or (index > 2)){
    return;
    }

  TCA9548A(ENC_ADRESS[index]);
  return angle(element_encoders[index], ANGLE_DISLOCATION[index]);
  }

void fix_servo_position(int index){
  if ((index < 0) or (index > 2)){
    return;
    }
  if (!are_enconers_connected){
    return;
    }
  fix_position(target_pos[index], enc_angle[index], element_steppers[index], SPIN_DIRECTION[index], REDUCTION[index]);
  }

void print_servo_position(int index){
  if ((index < 0) or (index > 2)){
    return;
    }

  stepper_print(element_steppers[index], enc_angle[index], REDUCTION[index]);
  }

void print_all_info(){
  if (!are_enconers_connected){
    Serial.println("ENCODERS WERE NOT CONNECTED!");
    return;
    }
  print_servo_position(0);
  print_servo_position(1);
  print_servo_position(2);
  print_target_coords();
  print_target_positions();
  }

void send_check_to_pc(){
  if ((abs(prev_pos[0] - current_position(enc_angle[0], REDUCTION[0])) <= DELTA) and 
  (abs(prev_pos[1] - current_position(enc_angle[1], REDUCTION[1])) <= DELTA) and
  (abs(prev_pos[2] - current_position(enc_angle[2], REDUCTION[2])) <= DELTA)){
    return;
    }
  //
  if ((abs(current_position(enc_angle[0], REDUCTION[0]) - target_pos[0]) > DELTA) or
  (abs(current_position(enc_angle[1], REDUCTION[1]) - target_pos[1]) > DELTA) or
  (abs(current_position(enc_angle[2], REDUCTION[2]) - target_pos[2]) > DELTA)){
    return;
    }
  //
  prev_pos[0] = current_position(enc_angle[0], REDUCTION[0]);
  prev_pos[1] = current_position(enc_angle[1], REDUCTION[1]);
  prev_pos[2] = current_position(enc_angle[2], REDUCTION[2]);
  Serial.println("ready");
  }

//main
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

  send_check_to_pc();
  //print_all_info();

  delay(40);
  }
