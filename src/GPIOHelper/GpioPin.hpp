// Copyright 2013 HTS-Software Unternehmensberatung
//                      Alexander Schucha
//
// This file is part of Athena-Api.
//
// Athena-Api is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Athena-Api is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with Athena-Api.  If not, see <http://www.gnu.org/licenses/>.
//
// https://git.hts-software.de/cgit.cgi/Athena-Api/tree/Gpio/Gpio.hpp
//
// Changed bei punky

#pragma once
#ifndef GPIOPIN_H
#define GPIOPIN_H

#include <functional>
#include <thread>
#include <fstream>

enum class pin_direction : int{
  in,
  out
};

enum class pin_trigger : int{
  none,
  falling,
  rising,
  both
};

enum class pin_value : int {
	off,
	on
};

std::ostream& operator<<(std::ostream& os, pin_direction c);
std::ostream& operator<<(std::ostream& os, pin_trigger c);
std::ostream& operator<<(std::ostream& os, pin_value c);

/**
 * CallBack delegate for Pin Value Change
 * @param port
 *    the Pin number as Text sample "1" or "23"
 * @param value
 *    the pin vaule
 */
typedef std::function<void(const std::string& port, const int& value)> pin_change_delegate;

/**
  * \ingroup SystemFunctions
  *
  * GpioPin Control an I/O PIN
  */
class GpioPin {
    std::string _port;
    pin_direction _direction;
    pin_trigger _trigger;
    std::fstream _filePortValue{};
    // Trigger Behandlung
    //triggerGpio* msgIntGpio;
    // Thread zum prüfen ob ein Interrupt am Port ausgelöst wurde.
    std::thread _ioWatchThread;
    // iNotify File und Kernel Msg Registrierung
    int _fileInotify{};
    int _msgInotify{};
    void CheckTrigger();
    pin_change_delegate _callback;
    bool _threadRun;
public:
    /**
     * Create new I/O Pin Class
     * @param port
     *    the Pin number as Text sample "1" or "23"
     * @param direction
     *    see pin_direction
     * @param trigger
     *    fire event when value changed ?
     *    see pin_trigger
     */
    explicit GpioPin(const std::string& port, pin_direction direction, pin_trigger trigger = pin_trigger::none);
    explicit GpioPin(std::uint8_t port, bool output);
    GpioPin(const GpioPin& orig) = delete;
    GpioPin(GpioPin&& other) = delete;
    GpioPin& operator=(const GpioPin& other) = delete;
    GpioPin& operator=(GpioPin&& other) = delete;
    virtual ~GpioPin();
    void operator<< (int iValue);
    void operator<< (pin_value value);
    void operator>> (int& iValue);
    void Register(const pin_change_delegate& callback);
    pin_direction GetDirection() const;

};

#endif /* GPIOPIN_H */

