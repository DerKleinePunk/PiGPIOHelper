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
  * File:   MCP23017.h
  * Author: punky
  *
  * Created on 20. November 2018
  */

#pragma once
#include "I2CDevice.hpp"

enum class pin_value;
enum class pin_direction;
class I2CBus;

/**
  * \ingroup SystemFunctions
  *
  * MCP23017
  */
class MCP23017 {
	I2CDevice* _device;
public:
	/**
	 * Create new MCP23017 Class to Controll the Chip via I²C
	 * @param bus
	 *    the bus see I2CBus
	 * @param deviceAddr
	 *    The device Address
	 */
	explicit MCP23017(I2CBus* bus, unsigned char deviceAddr);
	MCP23017(const MCP23017& orig) = delete;
	MCP23017(MCP23017&& other) = delete;
	MCP23017& operator=(const MCP23017& other) = delete;
	MCP23017& operator=(MCP23017&& other) = delete;
	virtual ~MCP23017();

	/**
	 * Contig Pin for Output or Input
	 * @param pin
	 *    the pin as number text
	 * @param direction
	 *    see pin_direction
	 */
	int ConfigPin(unsigned char pin, pin_direction direction) const;
	/**
	 * Set the pin if it is an output pin
	 * @param pin
	 *    the pin as number text
	 * @param valuePin
	 *    see pin_value
	 */
	int SetPin(unsigned char pin, const pin_value valuePin) const;
	int GetPin(unsigned char pin, pin_value& valuePin) const;
};
