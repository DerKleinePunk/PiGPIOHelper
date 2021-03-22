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
 * File:   I2CDevice.h
 * Author: punky
 *
 * Created on 20. November 2018
 */

#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "I2CDevice"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "I2CDevice.hpp"
#include "../common/easylogging/easylogging++.h"
#include "../common/exception/ConfigErrorException.hpp"
#include "../common/exception/NullPointerException.hpp"

I2CDevice::I2CDevice(I2CBus* bus, const unsigned char deviceAddr)
{
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    if(bus == nullptr) {
        throw NullPointerException("bus");
    }
    _bus = bus;
    _deviceAddr = deviceAddr;
}

I2CDevice::~I2CDevice()
{
}

/** Read multiple bits from an 8-bit device register.
 * @param regAddr Register regAddr to read from
 * @param bitStart First bit position to read (0-7)
 * @param length Number of bits to read (not more than 8)
 * @param value Container for right-aligned value (i.e. '101' read from any bitStart position will equal 0x05)
 * @return Status of read operation (0 < failed)
 */
int I2CDevice::ReadBits(unsigned char regAddr, unsigned char bitStart, unsigned char length, unsigned char& value)
{
    return _bus->ReadBits(_deviceAddr, regAddr, bitStart, length, value);
}

int I2CDevice::ReadByte(const unsigned char regAddr, unsigned char& value) const
{
    return _bus->ReadByte(_deviceAddr, regAddr, value);
}

int I2CDevice::ReadBytes(unsigned char regAddr, unsigned char length, unsigned char* value)
{
    return _bus->ReadBytes(_deviceAddr, regAddr, length, value);
}

int I2CDevice::WriteBit(unsigned char regAddr, unsigned char bitNum, unsigned char value)
{
    return _bus->WriteBit(_deviceAddr, regAddr, bitNum, value);
}

int I2CDevice::WriteBits(unsigned char regAddr, unsigned char bitStart, unsigned char length, unsigned char value)
{
    return _bus->WriteBits(_deviceAddr, regAddr, bitStart, length, value);
}

int I2CDevice::WriteByte(const unsigned char regAddr, const unsigned char value) const
{
    return _bus->WriteByte(_deviceAddr, regAddr, value);
}
