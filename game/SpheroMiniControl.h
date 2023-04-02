#ifndef SPHERO_MINI_CONTROL_H
#define SPHERO_MINI_CONTROL_H

#include <vector>

#include "Arduino.h"
#include "ArduinoBLE.h"
#include "String.h"
#include "Arduino_BHY2.h"

typedef std::vector<byte> byte_vector;

class SpheroMiniControl
{
private:
    String target_device_name;
    BLEDevice device;
    BLECharacteristic uart;
    byte sequence_id = 0;

public:
    SpheroMiniControl(String target_device_name = "SM-5935");

    void begin();

    void loop(SensorXYZ &accel, SensorXYZ &gyro);

private:
    void connect();

    void color(const byte red, const byte green, const byte blue);

    void set_aiming_led(bool active);

    void set_stabilization(bool active);

    void move(const byte speed, const uint direction);

    void execute(const bool command_success,
                 const String command_statement = "Unknwon command");

    bool send_raw(BLECharacteristic &characteristic, byte_vector &commands,
                  const bool auto_adjust = true);
};

#endif
