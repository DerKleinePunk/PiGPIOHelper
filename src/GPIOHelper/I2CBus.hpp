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

#pragma once
#include <mutex>

/**
 * \ingroup SystemFunctions
 *
 * I2CBus
 */
class I2CBus
{
    int _i2cBusHandle{};
    std::mutex _mtx;

  public:
    /**
     * Create new I2CBus
     * @param device
     *    string to device tree sample /DEV/I2C-1
     */
    explicit I2CBus(std::string device);
    I2CBus(const I2CBus& orig) = delete;
    I2CBus(I2CBus&& other) = delete;
    I2CBus& operator=(const I2CBus& other) = delete;
    I2CBus& operator=(I2CBus&& other) = delete;
    virtual ~I2CBus();

    int ReadBits(unsigned char deviceAddr,
                 unsigned char regAddr,
                 unsigned char bitStart,
                 unsigned char length,
                 unsigned char& value);
    int ReadByte(unsigned char deviceAddr, unsigned char regAddr, unsigned char& value);
    int ReadBytes(unsigned char deviceAddr, unsigned char regAddr, unsigned char length, unsigned char* value);

    int WriteBit(unsigned char deviceAddr, unsigned char regAddr, unsigned char bitNum, unsigned char value);
    int WriteBits(unsigned char deviceAddr,
                 unsigned char regAddr,
                 unsigned char bitStart,
                 unsigned char length,
                 unsigned char value);
    int WriteByte(unsigned char deviceAddr, unsigned char regAddr, unsigned char value);
};
