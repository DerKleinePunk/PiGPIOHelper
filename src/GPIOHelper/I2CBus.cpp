/*
 * Copyright (C) 2018 punky
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * File:   I2CBus.h
 * Author: punky
 *
 * Created on 20. November 2018
 */

// https://www.mjmwired.net/kernel/Documentation/i2c/dev-interface
// https://github.com/DavidEGrayson/minimu9-ahrs/blob/master/i2c_bus.cpp

#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "I2CBus"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "I2CBus.hpp"
#include <fcntl.h> //Needed for I2C port
#include <linux/i2c-dev.h> //Needed for I2C port
#include <linux/i2c.h>
#include <sys/ioctl.h> //Needed for I2C port
#include <unistd.h> //Needed for I2C port
#include <iostream>
#include "../common/easylogging/easylogging++.h"
#include "../common/exception/ConfigErrorException.hpp"

/**
 * @brief Construct a new I2CBus::I2CBus object
 * 
 * @param device 
 */
I2CBus::I2CBus(std::string device)
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _i2cBusHandle = open(device.c_str(), O_RDWR);

    if(_i2cBusHandle < 0) {
        LOG(ERROR) << device << "Port open Failed";
        std::string errmsg = device + std::string(" Port open Failed");
        throw ConfigErrorException(errmsg);
    }
}

I2CBus::~I2CBus()
{
    if(_i2cBusHandle > 0) {
        close(_i2cBusHandle);
    }
}

/** @brief Read multiple bits from an 8-bit device register.
 * @param deviceAddr I2C slave device address
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param value Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @return Status of read operation (0 < failed)
 */
int I2CBus::ReadBits(unsigned char deviceAddr,
                     unsigned char regAddr,
                     unsigned char bitStart,
                     unsigned char length,
                     unsigned char& value)
{
    unsigned char tempValue;
    auto result = ReadByte(deviceAddr, regAddr, tempValue);
    if(tempValue < 0) {
        value = 0x00;
        return result;
    }

    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    tempValue &= mask;
    tempValue >>= (bitStart - length + 1);
    value = tempValue;

    return result;
}

int I2CBus::ReadByte(const unsigned char deviceAddr, const unsigned char regAddr, unsigned char& value)
{
    if(_i2cBusHandle < 0) return -9;

    _mtx.lock();
    unsigned char outbuff;
    struct i2c_msg messages[2];

    outbuff = regAddr;
    messages[0].addr = deviceAddr;
    messages[0].flags = 0;
    messages[0].len = sizeof(outbuff);
    messages[0].buf = &outbuff;

    auto inbuff = &value;
    messages[1].addr = deviceAddr;
    messages[1].flags = I2C_M_RD;
    messages[1].len = sizeof(*inbuff);
    messages[1].buf = inbuff;

    struct i2c_rdwr_ioctl_data packets {
        .msgs = messages, .nmsgs = 2
    };


    const auto retVal = ioctl(_i2cBusHandle, I2C_RDWR, &packets);
    if(retVal < 0) LOG(ERROR) << "Read from I2C Device failed";
    _mtx.unlock();

    return retVal;
}

int I2CBus::ReadBytes(unsigned char deviceAddr, unsigned char regAddr, unsigned char length, unsigned char* value)
{
    if(_i2cBusHandle < 0) return -9;

    _mtx.lock();
    unsigned char outbuff;
    struct i2c_msg messages[2];

    outbuff = regAddr;
    messages[0].addr = deviceAddr;
    messages[0].flags = 0;
    messages[0].len = sizeof(outbuff);
    messages[0].buf = &outbuff;

    messages[1].addr = deviceAddr;
    messages[1].flags = I2C_M_RD;
    messages[1].len = length;
    messages[1].buf = value;

    struct i2c_rdwr_ioctl_data packets {
        .msgs = messages, .nmsgs = 2
    };


    const auto retVal = ioctl(_i2cBusHandle, I2C_RDWR, &packets);
    if(retVal < 0) LOG(ERROR) << "Read from I2C Device failed";
    _mtx.unlock();

    return retVal;
}

int I2CBus::WriteBit(unsigned char deviceAddr, unsigned char regAddr, unsigned char bitNum, unsigned char value)
{
    unsigned char currentValue = 0x00;
    auto result = ReadByte(deviceAddr, regAddr, currentValue);
    if(result < 0) return result;

    currentValue = (value != 0) ? (currentValue | (1 << bitNum)) : (currentValue & ~(1 << bitNum));

    return WriteByte(deviceAddr, regAddr, currentValue);
}

int I2CBus::WriteBits(unsigned char deviceAddr, unsigned char regAddr, unsigned char bitStart, unsigned char length, unsigned char value)
{
    unsigned char currentValue = 0x00;
    auto result = ReadByte(deviceAddr, regAddr, currentValue);
    if(result < 0) return result;

    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    value <<= (bitStart - length + 1); // shift data into correct position
    value &= mask; // zero all non-important bits in data
    currentValue &= ~(mask); // zero all important bits in existing byte
    currentValue |= value; // combine data with existing byte

    return WriteByte(deviceAddr, regAddr, currentValue);

}

int I2CBus::WriteByte(const unsigned char deviceAddr, const unsigned char regAddr, const unsigned char value)
{
    if(_i2cBusHandle < 0) return -9;

    _mtx.lock();
    unsigned char buff[2];
    struct i2c_msg messages[1];

    buff[0] = regAddr;
    buff[1] = value;

    messages[0].addr = deviceAddr;
    messages[0].flags = 0;
    messages[0].len = sizeof(buff);
    messages[0].buf = buff;

    struct i2c_rdwr_ioctl_data packets {
        .msgs = messages, .nmsgs = 1
    };


    const auto retVal = ioctl(_i2cBusHandle, I2C_RDWR, &packets);
    if(retVal < 0) LOG(ERROR) << "Write to I2C Device failed";

    _mtx.unlock();

    return retVal;
}
