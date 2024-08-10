#include "AS5600.h"                          

AS5600 as5600;                               

const double angle_1_dislocation = -5.27 + 180.0;
double angle_1 = 0.0;

void setup()
{
  Serial.begin(115200);

  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin();

  as5600.begin();
  int b = as5600.isConnected();
  Serial.print("Connect: ");
  Serial.println(b);
  delay(100);
}


void loop()
{
  angle_1 = double(as5600.rawAngle()) / 4096 * 360;
  Serial.print(angle_1);
  Serial.print('\t');
  Serial.println(angle_1-angle_1_dislocation);
  delay(25);

}
