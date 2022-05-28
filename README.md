# Raspberry GPIO Helpers

## How to build

## Using

### GPIOPin Simple

```cpp
  auto pin1 = new GpioPin("1", true);
  pin1 << 1; // Switch on
  pin1 << 0; // Switch off
```

### GPIOPin 
```cpp
  void callback(const std::string& port, const int& value)
  {

  }

  auto pin1 = new GpioPin("1", pin_direction::in, pin_trigger::falling);
  pin1.Register(callback);

```

Callback is Called wenn pin input Falling
## I²C Tests

i2cdetect -y 1 -> Bus Scan

ds3231 has also 0x68 Base Adresse be carefull (GY512/MPU-6050)
set AD0 to High
