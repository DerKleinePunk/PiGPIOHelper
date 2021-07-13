#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MPU5060"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

//Based On https://github.com/jrowberg/i2cdevlib/blob/master/RaspberryPi_bcm2835/MPU6050/MPU6050.cpp
//and https://github.com/jarzebski/Arduino-MPU6050/blob/master/MPU6050.cpp

#include "MPU5060.hpp"
#include "../common/easylogging/easylogging++.h"
#include "../common/exception/ConfigErrorException.hpp"

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

MPU5060::MPU5060(I2CBus* bus, unsigned char deviceAddr):
    _dpsPerDigit(0.0),
    _rangePerDigit(0.0)
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
    const auto result =
    _device->ReadBits(MPU6050_RA_WHO_AM_I, MPU6050_WHO_AM_I_BIT, MPU6050_WHO_AM_I_LENGTH, value);

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
    if(result <0) return;

    _dpsPerDigit = 1.0f;
    switch (range)
    {
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
            break;
    }
}

/** Set full-scale accelerometer range.
 * @param range New full-scale accelerometer range setting
 * @see getFullScaleAccelRange()
 */
void MPU5060::SetFullScaleAccelRange(const unsigned char range)
{
    _device->WriteBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT,
                       MPU6050_ACONFIG_AFS_SEL_LENGTH, range);
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
 * @see getAcceleration()
 * @see getRotation()
 * @see MPU6050_RA_ACCEL_XOUT_H
 */
void MPU5060::GetMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz)
{
    unsigned char buffer[14];

    _device->ReadBytes(MPU6050_RA_ACCEL_XOUT_H, 14, buffer);
    *ax = (((int16_t)buffer[0]) << 8) | buffer[1];
    *ay = (((int16_t)buffer[2]) << 8) | buffer[3];
    *az = (((int16_t)buffer[4]) << 8) | buffer[5];
    *gx = (((int16_t)buffer[8]) << 8) | buffer[9];
    *gy = (((int16_t)buffer[10]) << 8) | buffer[11];
    *gz = (((int16_t)buffer[12]) << 8) | buffer[13];
}

double MPU5060::GetTemp()
{
    unsigned char buffer[2];

    _device->ReadBytes(MPU6050_REG_TEMP_OUT_H, 2, buffer);
    int16_t value = (((int16_t)buffer[0]) << 8) | buffer[1];
    return (double)value / 340.0 + 36.53;
}