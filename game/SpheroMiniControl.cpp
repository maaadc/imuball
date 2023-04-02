#include "SpheroMiniControl.h"

SpheroMiniControl::SpheroMiniControl(const String target_device_name)
{
    this->target_device_name = target_device_name;
    return;
}

void SpheroMiniControl::begin()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    // initialize BLE
    if (!BLE.begin())
    {
        Serial.println("Starting BLE module failed.");
        while (1)
        {
        };
    }
    BLE.scan();

    Serial.println("BLE scanning ...");
}

void SpheroMiniControl::loop(SensorXYZ &accel, SensorXYZ &gyro)
{
    // check if a peripheral has been discovered
    this->device = BLE.available();

    if (this->device)
    {
        // see if peripheral is a LED
        if (this->device.localName() == this->target_device_name)
        {
            // stop scanning
            BLE.stopScan();

            this->connect();

            if (this->device && this->device.connected())
            {
                // blink green color at startup
                this->color(0x00, 0xff, 0x00);
                delay(1000);

                // prepare manual aiming
                this->color(0x00, 0x00, 0x00);
                this->set_stabilization(false);
                this->set_aiming_led(true);

                Serial.println("... manually rotate robot.");
                delay(5000);

                this->set_aiming_led(false);
                this->set_stabilization(true);
                this->color(0x80, 0x00, 0x00);
            }

            while (this->device && this->device.connected())
            {
                BHY2.update();

                // accel control
                float x = accel.x();
                float y = accel.y();
                float z = accel.z();
                float relative_tilt = sqrt(x * x + y * y) / sqrt(x * x + y * y + z * z);
                const uint speed = 255. * 0.5 * max(0.1, relative_tilt);

                // get angle in 0-360 deg range
                const uint angle = (atan2(y, x) / 3.14159 + 1.) * 180.;
                this->move(speed, angle);

                delay(50);
            }

            this->execute(this->device.disconnect(), "disconnect");
            // disconnected: Wait or start scanning again?
            while (true)
                ;
            BLE.scan();
        }
    }
}

void SpheroMiniControl::execute(const bool command_success,
                                const String command_statement)
{
    Serial.print("`");
    Serial.print(command_statement);
    Serial.print("` returned ");
    Serial.print(command_success);
    /*if (!command_success && this->device.connected()) {
      Serial.print(" ... disconnecting");
      this->device.disconnect();
    }*/
    Serial.println(" ");
}

void SpheroMiniControl::connect()
{
    // connect
    this->execute(this->device.connect(), "connect");
    this->execute(this->device.discoverAttributes(), "discoverAttributes");

    Serial.print("Connected to device `");
    Serial.print(this->device.localName());
    Serial.println("`");

    // send attach command
    BLECharacteristic attach_char =
        this->device.characteristic("00020005-574f-4f20-5370-6865726f2121");
    byte_vector attach_cmd{'u', 's', 'e', 't', 'h', 'e', 'f', 'o', 'r',
                           'c', 'e', '.', '.', '.', 'b', 'a', 'n', 'd'};
    this->send_raw(attach_char, attach_cmd, false);

    // setup of uart characteristic
    this->uart =
        this->device.characteristic("00010002-574f-4f20-5370-6865726f2121");

    // send wake command
    byte_vector wake_cmd{0x8d, 0x0a, 0x13, 0x0d, 0x00, 0x00, 0xd8};
    this->send_raw(this->uart, wake_cmd);

    // reset heading
    byte_vector heading_cmd{0x8d, 0x0a, 0x16, 0x06, 0x00, 0x00, 0xd8};
    this->send_raw(this->uart, heading_cmd);
}

void SpheroMiniControl::move(const byte speed, const uint direction)
{
    Serial.print("move(");
    Serial.print(speed);
    Serial.print(", ");
    Serial.print(direction);
    Serial.println(")");

    const byte head1 = (0xff00 & direction) >> 8;
    const byte head0 = (0x00ff & direction);
    byte_vector cmd{0x8d, 0x0a, 0x16, 0x07, 0x00, speed,
                    head1, head0, 0x00, 0x00, 0xd8};
    this->send_raw(this->uart, cmd);
}

void SpheroMiniControl::set_aiming_led(bool active)
{
    Serial.print("set_aiming_led(");
    Serial.print(active);
    Serial.println(")");

    byte brightness = (active) ? 0xff : 0x00;
    byte_vector cmd{0x8d, 0x0a, 0x1a, 0x0e, 0x00, 0x00, 0x01, brightness, 0x00, 0xd8};
    this->send_raw(this->uart, cmd);
}

void SpheroMiniControl::set_stabilization(bool active)
{
    Serial.print("set_stabilization(");
    Serial.print(active);
    Serial.println(")");

    byte value = active;
    byte_vector cmd{0x8d, 0x0a, 0x16, 0x0c, 0x00, value, 0x00, 0xd8};
    this->send_raw(this->uart, cmd);
}

void SpheroMiniControl::color(const byte red, const byte green,
                              const byte blue)
{
    Serial.print("color(");
    Serial.print(red);
    Serial.print(", ");
    Serial.print(green);
    Serial.print(", ");
    Serial.print(blue);
    Serial.println(")");

    byte_vector cmd{0x8d, 0x0a, 0x1a, 0x0e, 0x00, 0x00, 0x7e, red,
                    green, blue, red, green, blue, 0x00, 0xd8};
    this->send_raw(this->uart, cmd);
}

bool SpheroMiniControl::send_raw(BLECharacteristic &characteristic,
                                 byte_vector &commands,
                                 const bool auto_adjust)
/*  Sends specified byte vector via bluetooth.

    With `auto_adjust` the bytes for sequence id and checksum are
    automatically updated in the command vector prior to sending.
*/
{
    if (!characteristic.canWrite())
    {
        Serial.println("Characteristic is not writeable");
        return false;
    }

    if (auto_adjust)
    {
        // set consecutive sequence id
        commands[4] = this->sequence_id;
        this->sequence_id++;

        // compute checksum byte
        byte checksum = 0;
        for (int k = 1; k < commands.size() - 2; k++)
        {
            checksum += commands[k];
        }
        checksum = 0xff - checksum;

        commands[commands.size() - 2] = (byte)checksum;
    }

    if (false) // only activate in debug
    {
        Serial.print("Writing data: ");
        for (byte x : commands)
        {
            Serial.print(x, HEX);
            Serial.print(" ");
        }
        Serial.println("");
    }

    if (!characteristic.writeValue(commands.data(), commands.size()))
    {
        Serial.println("Writing data `` ... failed.");
        return false;
    }

    return true;
}
