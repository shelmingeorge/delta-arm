#include "AS5600.h"
#include "TI_TCA9548A.h"
#include "Wire.h"

byte enc_adress[] = {5, 6, 7};

AS5600 enc0;
AS5600 enc1;
AS5600 enc2;

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

void setup() {
  Serial.begin(115200);

  Wire.begin();

  TCA9548A(enc_adress[0]);
  encoder_setup(enc0);

  TCA9548A(enc_adress[1]);
  encoder_setup(enc1);

  TCA9548A(enc_adress[2]);
  encoder_setup(enc2);

  delay(1000);
}

void loop() {

  TCA9548A(enc_adress[0]);
  Serial.print(double(enc0.rawAngle()) / 4096 * 360);

  Serial.print("\t");

  TCA9548A(enc_adress[1]);
  Serial.print(double(enc1.rawAngle()) / 4096 * 360);

  Serial.print("\t");

  TCA9548A(enc_adress[2]);
  Serial.println(double(enc2.rawAngle()) / 4096 * 360);

  delay(25);
}
