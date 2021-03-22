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

#pragma once
#include "I2CBus.hpp"

/**
 * \ingroup SystemFunctions
 *
 * I2CDevice
 */
class I2CDevice
{
    unsigned char _deviceAddr;
    I2CBus* _bus;

  public:
    /**
     * Create new I2CDevice
     * @param bus
     *    see I2CBus
     * @param deviceAddr
     *    the device adresse 0 - 127
     */
    I2CDevice(I2CBus* bus, unsigned char deviceAddr);
    I2CDevice(const I2CDevice& orig) = delete;
    I2CDevice(I2CDevice&& other) = delete;
    I2CDevice& operator=(const I2CDevice& other) = delete;
    I2CDevice& operator=(I2CDevice&& other) = delete;
    ~I2CDevice();

    int ReadBits(unsigned char regAddr,
                 unsigned char bitStart,
                 unsigned char length,
                 unsigned char& value);
    int ReadByte(unsigned char regAddr, unsigned char& value) const;
    int ReadBytes(unsigned char regAddr, unsigned char length, unsigned char* value);
    
    int WriteBit(unsigned char regAddr, unsigned char bitNum, unsigned char value);
    int WriteBits(unsigned char regAddr,
                 unsigned char bitStart,
                 unsigned char length,
                 unsigned char value);
    int WriteByte(unsigned char regAddr, unsigned char value) const;
};
