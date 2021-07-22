#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MPU5060"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

// Based On https://github.com/jrowberg/i2cdevlib/blob/master/RaspberryPi_bcm2835/MPU6050/MPU6050.cpp
// and https://github.com/jarzebski/Arduino-MPU6050/blob/master/MPU6050.cpp
// and https://github.com/rfetick/MPU6050_light/blob/master/src/MPU6050_light.cpp

#include "MPU5060.hpp"
#include <cmath>
#include "../common/easylogging/easylogging++.h"
#include "../common/exception/ConfigErrorException.hpp"

#define RAD_2_DEG 57.29578 // 180 / M_PI [deg/rad]
#define DEFAULT_GYRO_COEFF 0.98

#define MPU6050_RA_WHO_AM_I 0x75
#define MPU6050_WHO_AM_I_BIT 6
#define MPU6050_WHO_AM_I_LENGTH 6

#define MPU6050_CLOCK_INTERNAL 0x00
#define MPU6050_CLOCK_PLL_XGYRO 0x01
#define MPU6050_CLOCK_PLL_YGYRO 0x02
#define MPU6050_CLOCK_PLL_ZGYRO 0x03
#define MPU6050_CLOCK_PLL_EXT32K 0x04
#define MPU6050_CLOCK_PLL_EXT19M 0x05
#define MPU6050_CLOCK_KEEP_RESET 0x07

#define MPU6050_GYRO_FS_250 0x00
#define MPU6050_GYRO_FS_500 0x01
#define MPU6050_GYRO_FS_1000 0x02
#define MPU6050_GYRO_FS_2000 0x03
#define MPU6050_RA_GYRO_CONFIG 0x1B

#define MPU6050_GCONFIG_FS_SEL_BIT 4
#define MPU6050_GCONFIG_FS_SEL_LENGTH 2

#define MPU6050_ACONFIG_AFS_SEL_BIT 4
#define MPU6050_ACONFIG_AFS_SEL_LENGTH 2

#define MPU6050_ACCEL_FS_2 0x00
#define MPU6050_ACCEL_FS_4 0x01
#define MPU6050_ACCEL_FS_8 0x02
#define MPU6050_ACCEL_FS_16 0x03
#define MPU6050_RA_ACCEL_CONFIG 0x1C
#define MPU6050_RA_PWR_MGMT_1 0x6B
#define MPU6050_RA_PWR_MGMT_2 0x6C

#define MPU6050_PWR1_SLEEP_BIT 6

#define MPU6050_RA_ACCEL_XOUT_H 0x3B
#define MPU6050_RA_ACCEL_XOUT_L 0x3C
#define MPU6050_RA_ACCEL_YOUT_H 0x3D
#define MPU6050_RA_ACCEL_YOUT_L 0x3E
#define MPU6050_RA_ACCEL_ZOUT_H 0x3F
#define MPU6050_RA_ACCEL_ZOUT_L 0x40
#define MPU6050_REG_TEMP_OUT_H 0x41

/* Wrap an angle in the range [-limit,+limit] (special thanks to Edgar Bonet!) */
static float wrap(float angle, float limit)
{
    while(angle > limit)
        angle -= 2 * limit;
    while(angle < -limit)
        angle += 2 * limit;
    return angle;
}

MPU5060::MPU5060(I2CBus* bus, unsigned char deviceAddr)
    : _dpsPerDigit(0.0), _rangePerDigit(0.0), _upsideDownMounting(false), _filterGyroCoef(DEFAULT_GYRO_COEFF), _lastTime(timer::now()), _angleX(0.0),_angleY(0.0),_angleZ(0.0)
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _device = new I2CDevice(bus, deviceAddr);
}

MPU5060::~MPU5060()
{
}

bool MPU5060::InitDevice()
{
    unsigned char value = 0x00;
    const auto result = _device->ReadBits(MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, value);

    if(result < 0 || value != 0x34) {
        LOG(ERROR) << "no mpu6050 or i2c read error result " << result << " value " << value;
        return false;
    }

    SetClockSource(MPU6050_CLOCK_PLL_XGYRO);
    SetFullScaleGyroRange(MPU6050_GYRO_FS_250);
    SetFullScaleAccelRange(MPU6050_ACCEL_FS_2);
    SetSleepEnabled(false);

    return true;
}

void MPU5060::SetClockSource(const unsigned char source)
{
    _device->WriteBits(MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, source);
}

/** Set full-scale gyroscope range.
 * @param range New full-scale gyroscope range value
 * @see getFullScaleRange()
 * @see MPU6050_GYRO_FS_250
 * @see MPU6050_RA_GYRO_CONFIG
 * @see MPU6050_GCONFIG_FS_SEL_BIT
 * @see MPU6050_GCONFIG_FS_SEL_LENGTH
 */
void MPU5060::SetFullScaleGyroRange(const unsigned char range)
{
    const auto result = _device->WriteBits(MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range);
    if(result < 0) {
        LOG(ERROR) << "Error Writing I²C";

        return;
    }

    _dpsPerDigit = 1.0f;
    switch(range) {
    case MPU6050_GYRO_FS_250:
        _dpsPerDigit = .007633f;
        break;
    case MPU6050_GYRO_FS_500:
        _dpsPerDigit = .015267f;
        break;
    case MPU6050_GYRO_FS_1000:
        _dpsPerDigit = .030487f;
        break;
    case MPU6050_GYRO_FS_2000:
        _dpsPerDigit = .060975f;
        break;
    default:
        LOG(ERROR) << "Config Value Out Offrange";
        break;
    }
}

/** Set full-scale accelerometer range.
 * @param range New full-scale accelerometer range setting
 * @see getFullScaleAccelRange()
 * @see MPU6050_ACCEL_FS_2
 * @see MPU6050_ACCEL_FS_4
 * @see MPU6050_ACCEL_FS_8
 * @see MPU6050_ACCEL_FS_16
 */
void MPU5060::SetFullScaleAccelRange(const unsigned char range)
{
    const auto result = _device->WriteBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range);

    if(result < 0) {
        LOG(ERROR) << "Error Writing I²C";
        return;
    }

    _rangePerDigit = 0.0f;
    switch(range) {
    case MPU6050_ACCEL_FS_2:
        _rangePerDigit = .000061; // => 1 : 16384.0
        break;
    case MPU6050_ACCEL_FS_4:
        _rangePerDigit = .000122; // => 1 : 8192.0
        break;
    case MPU6050_ACCEL_FS_8:
        _rangePerDigit = .000244;
        break;
    case MPU6050_ACCEL_FS_16:
        _rangePerDigit = .0004882;
        break;
    default:
        LOG(ERROR) << "Config Value Out Offrange";
        break;
    }
}

void MPU5060::SetSleepEnabled(bool enabled)
{
    _device->WriteBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled);
}

/** Get raw 6-axis motion sensor readings (accel/gyro).
 * Retrieves all currently available motion sensor values.
 * @param ax 16-bit signed integer container for accelerometer X-axis value
 * @param ay 16-bit signed integer container for accelerometer Y-axis value
 * @param az 16-bit signed integer container for accelerometer Z-axis value
 * @param gx 16-bit signed integer container for gyroscope X-axis value
 * @param gy 16-bit signed integer container for gyroscope Y-axis value
 * @param gz 16-bit signed integer container for gyroscope Z-axis value
 * @param temp 16-bit signed integer container for gyroscope Z-axis value
 * @see getAcceleration()
 * @see getRotation()
 * @see MPU6050_RA_ACCEL_XOUT_H
 */
void MPU5060::GetRawMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz, int16_t* temp)
{
    unsigned char buffer[14];

    _device->ReadBytes(MPU6050_RA_ACCEL_XOUT_H, 14, buffer);

    *ax = (((int16_t)buffer[0]) << 8) | buffer[1];
    *ay = (((int16_t)buffer[2]) << 8) | buffer[3];
    *az = (((int16_t)buffer[4]) << 8) | buffer[5];

    *temp = (((int16_t)buffer[6]) << 8) | buffer[7];

    *gx = (((int16_t)buffer[8]) << 8) | buffer[9];
    *gy = (((int16_t)buffer[10]) << 8) | buffer[11];
    *gz = (((int16_t)buffer[12]) << 8) | buffer[13];
}

void MPU5060::GetMotion6(double* accX, double* accY, double* accZ, double* gyroX, double* gyroY, double* gyroZ, double* tempC)
{
    int16_t ax, ay, az, gx, gy, gz, temp;

    GetRawMotion6(&ax, &ay, &az, &gx, &gy, &gz, &temp);

    *accX = static_cast<double>(ax) * _rangePerDigit;
    *accY = static_cast<double>(ay) * _rangePerDigit;
    *accZ = static_cast<double>(az) * _rangePerDigit;

    if(_upsideDownMounting) *accZ = -*accZ;

    *gyroX = static_cast<double>(gx) * _dpsPerDigit;
    *gyroY = static_cast<double>(gy) * _dpsPerDigit;
    *gyroZ = static_cast<double>(gz) * _dpsPerDigit;

    *tempC = static_cast<double>(temp) / 340.0 + 36.53;
}

double MPU5060::GetTemp()
{
    unsigned char buffer[2];

    _device->ReadBytes(MPU6050_REG_TEMP_OUT_H, 2, buffer);
    int16_t value = (((int16_t)buffer[0]) << 8) | buffer[1];
    return static_cast<double>(value) / 340.0 + 36.53;
}

//https://gist.github.com/arvind-iyer/f146533f370960366e11a84cbce45bb5
void MPU5060::GetAngels(double* angleAccX, double* angleAccY, double* angleX, double* angleY, double* angleZ)
{
    double accX, accY, accZ, gyroX, gyroY, gyroZ, temp;
    GetMotion6(&accX, &accY, &accZ, &gyroX, &gyroY, &gyroZ, &temp);

    // estimate tilt angles: this is an approximation for small angles!
    double sgZ = (accZ >= 0) - (accZ < 0); // allow one angle to go from -180 to +180 degrees
    *angleAccX = atan2(accY, sgZ * sqrt(accZ * accZ + accX * accX)) * RAD_2_DEG; // [-180,+180] deg
    *angleAccY = -atan2(accX, sqrt(accZ * accZ + accY * accY)) * RAD_2_DEG; // [- 90,+ 90] deg
    double angleAccZ = 0;  //Accelerometer doesn't give z-angle

    /*Todo
    unsigned long Tnew = millis();
    float dt = (Tnew - preInterval) * 1e-3; //0.001
    preInterval = Tnew;*/

    const auto now = timer::now();
    double dt = std::chrono::duration_cast<std::chrono::microseconds>(now - _lastTime).count();
    _lastTime = now;

    // Compute the (filtered) gyro angles
    double gyro_angle_x = gyroX*dt + get_last_x_angle();
    double gyro_angle_y = gyroY*dt + get_last_y_angle();
    double gyro_angle_z = gyroZ*dt + get_last_z_angle();

    // Correctly wrap X and Y angles (special thanks to Edgar Bonet!)
    // https://github.com/gabriel-milan/TinyMPU6050/issues/6
    /*_angleX = wrap(_filterGyroCoef * (*angleAccX + wrap(_angleX + gyroX * dt - *angleAccX, 180)) + (1.0 - _filterGyroCoef) * *angleAccX, 180);
    _angleY = wrap(_filterGyroCoef * (*angleAccY + wrap(_angleY + sgZ * gyroY * dt - *angleAccY, 90)) + (1.0 - _filterGyroCoef) * *angleAccY, 90);
    _angleZ += gyroZ * dt; // not wrapped (to do???)*/

    *angleX = _angleX;
    *angleY = _angleY;
    *angleZ = _angleZ;
}