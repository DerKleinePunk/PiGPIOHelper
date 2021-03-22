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
#   define ELPP_DEFAULT_LOGGER "SoftwarePWM"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include "../common/easylogging/easylogging++.h"
#include "SoftwarePWM.hpp"
#include <thread>
#include "../common/utils/ElapsedTimer.hpp"
#include "GpioPin.hpp"
#include <iostream>

void SoftwarePWM::PwmTrigger() const {
    el::Helpers::setThreadName("SoftwarePWM");
    ElapsedTimer timer(true);
    while(_threadRun) {
        *_pin << 1;
        std::this_thread::sleep_for(std::chrono::microseconds(_signalTime));
        *_pin << 0;
        std::this_thread::sleep_for(std::chrono::microseconds(_periodTime) - timer.Elapsed());
        timer.Reset();
    }
}

SoftwarePWM::SoftwarePWM(GpioPin* pin, const unsigned int frq, const unsigned int signal) {
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    LOG(DEBUG) << "Start SoftwarePWM Constructor ... ";
    _pin = pin;
    _periodTime = static_cast<unsigned long>(1.0 / frq * 1000000); // microsecond
    _threadRun = true;
    _signalTime = static_cast<unsigned long>(_periodTime * (signal / 10) / 100);
    LOG(DEBUG) << "signalTime is " << _signalTime;
    LOG(DEBUG) << "periodTime is " << _periodTime;
    _pwmThread = std::thread(&SoftwarePWM::PwmTrigger, this);
}

SoftwarePWM::~SoftwarePWM() {
    LOG(DEBUG) << "Start SoftwarePWM Destructor ... ";
    _threadRun = false;
    if (_pwmThread.joinable()) {
        _pwmThread.join();
    }
}

void SoftwarePWM::ChangeSignal(unsigned int signal) {
    _signalTime = static_cast<unsigned long>(_periodTime * (signal / 10) / 100);
    LOG(DEBUG) << "signalTime is " << _signalTime;
}
