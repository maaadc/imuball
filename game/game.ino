/*
  IMU BALL GAME
  Control the Sphero Mini with the IMU on the Arduino Nicla Sense ME
*/

#include "Arduino_BHY2.h"
#include "SpheroMiniControl.h"
#include "Nicla_System.h"

auto ball = SpheroMiniControl("SM-5935");
SensorXYZ accel(SENSOR_ID_ACC);
SensorXYZ gyro(SENSOR_ID_GYRO);

void setup()
{
    nicla::begin();
    // increase battery charging current
    nicla::enableCharge(100);

    BHY2.begin();
    accel.begin();
    gyro.begin();

    ball.begin();
}

void loop()
{
    ball.loop(accel, gyro);
}
