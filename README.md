# Raspberry GPIO Helpers

GPIO access via sysfs likely to be dropped
see https://github.com/WiringPi/WiringPi/issues/186
so Pin at now work only with Kernel < 6.6

## How to build

mkdir build
cd build
cmake ..
cmake . --build

This Repro is to using als git module.
only Samples are here not an working app

## Using

### GPIO Output Pin

```cpp
  auto pin1 = new GpioPin("1", true);
  pin1 << 1; // Switch on
  pin1 << 0; // Switch off
```

### GPIO Input Pin

```cpp
  void callback(const std::string& port, const int& value)
  {

  }

  auto pin1 = new GpioPin("1", pin_direction::in, pin_trigger::falling);
  pin1.Register(callback);

```

Callback is Called wenn pin input Falling

### MCP23017 on I²C

```cpp
   auto i2cBus = new I2CBus("/dev/i2c-1");
   auto mcpImpl = new MCP23017(i2cBus, 0x20);
   mcpImpl.SetPin(0, pin_value::on); // Set the 0 Pin On

```

### MPU5060 on I²C (not working ok)

```cpp
    auto i2cBus = new I2CBus("/dev/i2c-1");
    auto mpu = new MPU5060(i2cBus, 0x69);
```

## I²C Tests

i2cdetect -y 1 -> Bus Scan

ds3231 has also 0x68 Base Adresse be carefull (GY512/MPU-6050)
set AD0 to High
