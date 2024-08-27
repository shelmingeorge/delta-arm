const char default_string[] = "---------";
const char endl = 'e';
const char pause = 'p';
const char play = 'c';
const char grab = 'g';

bool is_grabbed = 0;
char input = '0';
String string = default_string;
long target_pos_uncut = 0;
int target_fi = 180;
int target_dist = 100;
int target_height = 100;

byte i = 0;


void add_char(char input_char){
      switch (input_char)
    {
      case endl:
        break;

      case grab:
        if (is_grabbed){
          Serial.println("releasing");
          is_grabbed = 0;
        }
        else {
          Serial.println("grabbing");
          is_grabbed = 1;
        }
        break;
      
      case pause:
        i = 0;
        string = default_string;
        Serial.println("PRESS <C> TO CONTINUE");
        while(Serial.read() != play){
          delay(100);
        }
        Serial.println("THE PROGRAM CONTINIOUS");
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
  String string_fi = string;
  String string_dist = string;
  String string_height = string;
  string_fi.remove(3);
  string_dist.remove(0, 3);
  string_dist.remove(3);
  string_height.remove(0, 6);

  Serial.print(string_fi);
  Serial.print("\t");
  Serial.print(string_dist);
  Serial.print("\t");
  Serial.print(string_height);
  Serial.print("\t");
  Serial.print(string_height.toInt());
  Serial.print("\t");

  target_pos_uncut = string.toInt();
  Serial.print(target_pos_uncut);
  Serial.print("\t");

  if (target_pos_uncut / 1000 / 1000 <= 30){
    return;
  }
  if (target_pos_uncut / 1000 / 1000 >= 330){
    return;
  }
  target_fi = target_pos_uncut / 1000 / 1000;
  Serial.print(target_fi);
  Serial.print("\t");

  if (target_pos_uncut / 1000 % 1000 <= 0){
    return;
  }
  if (target_pos_uncut % 1000 % 1000 <= 0){
    return;
  }

  target_dist = target_pos_uncut / 1000 % 1000;
  Serial.print(target_dist);
  Serial.print("\t");

  target_height = target_pos_uncut % 1000 % 1000;
  Serial.println(target_height);

}


void setup() {
  Serial.begin(115200);

}


void loop() {
  check_input();
  
  if ((string != default_string) and (input == endl)){
    Serial.println(string);

    get_coords();

    string = default_string;
    i = 0;
  }

}
