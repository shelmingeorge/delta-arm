const String default_x = "no_data"; //any current data from arduino
String x = default_x;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(1);

}

void loop() {
  while (!Serial.available());
  x = Serial.readString()/*.toInt()*/;
  
  if (x == "w"){
    Serial.print(default_x);
  }
  else{
  Serial.print(x);
  }

  delay(20);

}
