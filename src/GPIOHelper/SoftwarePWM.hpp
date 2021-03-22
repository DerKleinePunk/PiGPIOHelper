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
  * File:   SoftwarePWM.h
  * Author: punky
  *
  * Created on 20. November 2018
  */

#pragma once
#include <thread>

class GpioPin;

/**
  * \ingroup SystemFunctions
  *
  * SoftwarePWM
  */
class SoftwarePWM {
    GpioPin* _pin;
    unsigned long _periodTime;
    unsigned long _signalTime;
    std::thread _pwmThread;
    bool _threadRun;
    unsigned char _pause{};

    void PwmTrigger(void) const;
public:
    /**
     * Create new SoftwarePWM Thread
     * @param pin
     *    the pin will conntroled by pwm thread
     * @param frq
     *    The Freqenz in Hz
     * @param signal
     *    signal length 0 - 1000 (0.0 - 100.0 %)
     */
    explicit SoftwarePWM(GpioPin* pin, const unsigned int frq, const unsigned int signal);
    SoftwarePWM(const SoftwarePWM& orig) = delete;
    SoftwarePWM(SoftwarePWM&& other) = delete;
    SoftwarePWM& operator=(const SoftwarePWM& other) = delete;
    SoftwarePWM& operator=(SoftwarePWM&& other) = delete;
    virtual ~SoftwarePWM();

    /**
     * Change Signal Length
     * @param signal
     *    signal length 0 - 1000 (0.0 - 100.0 %)
     */
    void ChangeSignal(unsigned int signal);
};
