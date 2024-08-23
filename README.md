# ESP 32 Pulse Sensor

This repository integrates the ESP32 with a pulse sensor.

# List of materials

1. ESP32.
2. Max30102 pulse sensor.
3. Protoboard.
4. Jumpers.

# Eletric schema

In this project, the `Max30102` pulse sensor is connected to the `ESP32` as shown on the following table:

| MAX30102 PIN | ESP32 PIN |
| --- | --- |
| VIN | 3V3 |
| GND | GND |
| SCL | GPIO 22 |
| SDA | GPIO 21 |

# Libraries

1. [SparkFun MAX3010x](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library): to manage Max30102 sensor.

# Credits

1. [SparkFun MAX3010x sample](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library/blob/master/examples/Example5_HeartRate/Example5_HeartRate.ino).