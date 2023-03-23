// Prints accel and gyro readings to serial monitor

#include "Arduino_BHY2.h"

SensorXYZ accel(SENSOR_ID_ACC);
SensorXYZ gyro(SENSOR_ID_GYRO);

void setup() {
  Serial.begin(115200);

  BHY2.begin();
  accel.begin();
  gyro.begin();
}

void loop() {
  static auto lastCheck = millis();

  // Update function should be continuously polled
  BHY2.update();

  // Check sensor values every second
  if (millis() - lastCheck >= 1000) {
    lastCheck = millis();

    Serial.println(String("acceleration: ") + accel.toString());
    Serial.println(String("gyroscope: ") + gyro.toString());
  }
}
