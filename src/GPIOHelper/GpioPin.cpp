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

#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "GpioPin"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include <cstring>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <chrono>
#include <thread>
#include "../common/easylogging/easylogging++.h"
#include "../common/exception/ConfigErrorException.hpp"
#include "GpioPin.hpp"

std::ostream& operator<<(std::ostream& os, const pin_direction c) {
	switch (c) {
          case pin_direction::in: os << "in";    break;
          case pin_direction::out: os << "out"; break;
          default:  os << "PinDirection not in list";
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const pin_trigger c) {
	switch (c) {
          case pin_trigger::none: os << "none";    break;
          case pin_trigger::falling: os << "falling"; break;
          case pin_trigger::rising: os << "rising"; break;
          case pin_trigger::both: os << "both"; break;
          default:  os << "PinTrigger not in list";
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const pin_value c) {
	switch (c) {
          case pin_value::on: os << "on";    break;
          case pin_value::off: os << "off";    break;
          default:  os << "pin_value not in list";
	}
	return os;
}

/**
 * @brief Construct a new Gpio Pin:: Gpio Pin object
 * 
 * @param port 
 * @param direction : "in", "out"
 * @param trigger trigger  : "none"    (kein Trigger wird ausgelöst) 
 *                           "falling" (fallende Flanke löst Trigger aus)
 *                           "rising"  (steigende Flanke löst Trigger aus)
 *                           "both"    (beide Flanken lösen Trigger aus)
 */
GpioPin::GpioPin(const std::string& port, pin_direction direction, pin_trigger trigger) {
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    _port = port;
    _direction = direction;
    _trigger = trigger;
    _threadRun = false;

    LOG(DEBUG) << "Create Pin Class for " << _port << " with " << _direction << " " << _trigger;

    std::ofstream filePortExport ("/sys/class/gpio/export");

    if (!filePortExport) {
		LOG(ERROR) << "Port " << _port << " konnte nicht registriert werden";
		throw ConfigErrorException("Port konnte nicht registriert werden");
    }

    filePortExport << _port;
    filePortExport.close ();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //We need wait a little rigths an so on

    // Port als Eingang/Ausgang konfigurieren
    std::string address = "/sys/class/gpio/gpio";
    address += _port;
    address += "/direction";
    std::ofstream filePortDirect(address);

    if (!filePortDirect) {
	    LOG(ERROR) << "Port konnte nicht als " << _direction << " konfiguriert werden";
        throw ConfigErrorException("Port konnte nicht konfiguriert werden");
    }

    filePortDirect << _direction;
    filePortDirect.close ();
    
    // Stream zum Port schalten öffnen.
    // Schreib/Lese Puffer auf länge Null stellen.
    _filePortValue.open ("/sys/class/gpio/gpio" + _port + "/value", std::fstream::in | std::fstream::out | std::fstream::binary);
    _filePortValue.rdbuf()->pubsetbuf(nullptr, 0);
      
    // Trigger für den Port konfigurieren oder ausschalten.
    std::ofstream filePortTrigger ("/sys/class/gpio/gpio" + _port + "/edge");
      
    if (!filePortTrigger) {
        LOG(ERROR) << "Port-Trigger konnte nicht als " << _trigger << " konfiguriert werden";
        throw ConfigErrorException("Port konnte nicht konfiguriert werden");
    }
        
    filePortTrigger << _trigger;
    filePortTrigger.close ();
      
    if (_trigger != pin_trigger::none)
    {
        // Thread zum prüfen auf Trigger Ereignis starten.
        _ioWatchThread = std::thread(&GpioPin::CheckTrigger, this);
    }
}

GpioPin::GpioPin(std::uint8_t port, bool output):
    GpioPin(std::to_string(port), output ? pin_direction::out : pin_direction::in, pin_trigger::none) {

}

GpioPin::~GpioPin() {
    LOG(DEBUG) << "Start Gpio Destructor ... ";

    if (_filePortValue.is_open()) {
        // Stream zum Port schalten schließen.
        _filePortValue.close();
    }
      
    // Port Registrierung wieder aufheben
    std::ofstream filePortUnexport ("/sys/class/gpio/unexport");
    filePortUnexport << _port;
    filePortUnexport.close ();

    if (_ioWatchThread.joinable()) {
        _threadRun = false;
        inotify_rm_watch(_fileInotify, _msgInotify);

        // Warten bis der Thread beendet wurde.
        _ioWatchThread.join();
        close(_fileInotify);
    }
}

// Port ein/aus schalten.
void GpioPin::operator<< (const int iValue) {
    if(_direction != pin_direction::out) {
        throw ConfigErrorException("we can write only on out pins");
    }
    _filePortValue << iValue << std::flush;
    //std::cout << "port value is " << iValue << std::endl;
}

void GpioPin::operator<< (pin_value value) {
    if(value == pin_value::on) {
        operator<<(1);
    } else if(value == pin_value::off) {
        operator<<(0);
    }
}
  
// Aktuellen Port Zustand ein/aus lesen.
void GpioPin::operator>> (int& iValue) {
    iValue = -1;

    char cZeichen;
    _filePortValue.seekg(0);
    _filePortValue.read(&cZeichen, 1);

    if (_filePortValue.eof()) {
        LOG(ERROR) << "GPIO-Port-Stream, void operator>> (int& iValue): nichts da zum lesen";
    }

    if (_filePortValue.good())
    {
        std::stringstream str;

        if (str << cZeichen)
            str >> iValue;
        else
            LOG(ERROR) << "ERROR string to int";
    }
    else
        LOG(WARNING) << "GPIO-Port-Stream, void operator>> (int& iValue): nix good";
}

void GpioPin::Register(const pin_change_delegate& callback) {
    _callback = callback;
}

pin_direction GpioPin::GetDirection() const
{
    return _direction;
}

// Prüfe ob ein Trigger am Port ausgelöst wurde.
void GpioPin::CheckTrigger() {
    // Kernel Benachrichtigung inotify: initialisieren
    #define EVENT_SIZE  ( sizeof (struct inotify_event) )
    #define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
	_threadRun = true;

    uint32_t i = 0;
    char buffer[BUF_LEN];
      
    _fileInotify = inotify_init1(IN_CLOEXEC);
      
    if (_fileInotify < 0) {
	LOG(ERROR) << "Error: inotify_init()";
        exit(0);
    }
      
    // Kernel Benachrichtigung für Datei registrieren
    auto strPfad = "/sys/class/gpio/gpio" + _port + "/value";
    _msgInotify = inotify_add_watch(_fileInotify, strPfad.c_str (), IN_MODIFY);
      
    while(_threadRun) {
	    const auto length = read(_fileInotify, buffer, BUF_LEN);

        if (length < 0) {
            LOG(ERROR) << "Error: read ()";
            break;
        }  

        while (i < length)
        {
	        const auto event = reinterpret_cast<struct inotify_event*>(&buffer[i]);

            if (event->len) {
                if (event->mask & IN_MODIFY) {
                    if (event->mask & IN_ISDIR) {
			            LOG(DEBUG) << "The directory " << event->name <<  " was modified.";
                    } else {
                        if(_callback != nullptr) {
                            int value;
                            *this >> value;
                            _callback(_port, value);
                        } else {
                            LOG(WARNING) << "Trigger is call but no callback";
                        }
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
        i = 0;
    }
}