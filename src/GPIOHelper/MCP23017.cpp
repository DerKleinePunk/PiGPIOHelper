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
  * File:   SoftwarePWM.cpp
  * Author: punky
  *
  * Created on 20. November 2018
  */

#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "MCP23017"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "../common/easylogging/easylogging++.h"
#include "../common/exception/ConfigErrorException.hpp"
#include <bitset>
#include "MCP23017.hpp"
#include "GpioPin.hpp"
#include <iostream>

MCP23017::MCP23017(I2CBus* bus, unsigned char deviceAddr) {
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _device = new I2CDevice(bus, deviceAddr);
}

MCP23017::~MCP23017() {
}

int MCP23017::ConfigPin(const unsigned char pin, const pin_direction direction) const {
    auto internalPin = pin;
    if (pin > 15) {
        LOG(ERROR) << "pin must be 0 - 15";
        return -9;
    }

    unsigned char regAddr = 0x00;
    if(internalPin > 7) {
        internalPin = static_cast<unsigned char>(internalPin - 8);
        regAddr = 0x01;
    }

    unsigned char value;
    auto result = _device->ReadByte(regAddr, value);
    if(result < 0) {
        LOG(ERROR) << "error read register";
        return result;
    }

    if(direction == pin_direction::in) {
        value |= static_cast<unsigned char>(1 << internalPin);
    } else {
        value &= static_cast<unsigned char>(~(1 << internalPin));
    }

    result = _device->WriteByte(regAddr, value);
    if (result < 0) {
        LOG(ERROR) << "error write register";
        return result;
    }

    LOG(DEBUG) << "New Value is " << std::bitset<8>(value);

    return 0;
}

int MCP23017::SetPin(const unsigned char pin, const pin_value valuePin) const {
    auto internalPin = pin;
    if (pin > 15) {
        LOG(ERROR) << "pin must be 0 - 15";
        return -9;
    }

    unsigned char regAddr = 0x14;
    if (internalPin > 7) {
        internalPin = static_cast<unsigned char>(internalPin - 8);
        regAddr = 0x15;
    }

    //Read the register to not change the other pins on this register
    unsigned char value;
    auto result = _device->ReadByte(regAddr, value);
    if (result < 0) {
        LOG(ERROR) << "error read register";
        return result;
    }

    if (valuePin == pin_value::on) {
        value |= static_cast<unsigned char>(1 << internalPin);
    }
    else {
        value &= static_cast<unsigned char>(~(1 << internalPin));
    }

    result = _device->WriteByte(regAddr, value);
    if (result < 0) {
        LOG(ERROR) << "error write register" << std::endl;
        return result;
    }

    LOG(DEBUG) << "New Value is " << std::bitset<8>(value).to_string();

    return 0;
}

int MCP23017::GetPin(unsigned char pin, pin_value& valuePin) const {
    auto internalPin = pin;
    if (pin > 15) {
        LOG(ERROR) << "pin must be 0 - 15";
        return -9;
    }

    unsigned char regAddr = 0x12;
    if (internalPin > 7) {
        internalPin = static_cast<unsigned char>(internalPin - 8);
        regAddr = 0x13;
    }

    unsigned char value;
    auto result = _device->ReadByte(regAddr, value);
    if (result < 0) {
        LOG(ERROR) << "error read register";
        return result;
    }

    auto bitresult = std::bitset<8>(value);
    if(bitresult.test(internalPin)) {
        valuePin = pin_value::on;
    } else {
        valuePin = pin_value::off;
    }

    LOG(DEBUG) << "Value is " << bitresult.to_string();
    
    return 0;
}
