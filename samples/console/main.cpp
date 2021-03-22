#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "Main"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#include <iostream>
#include <chrono>
#include <thread>

#include "../../src/GPIOHelper/MPU5060.hpp"
#include "../../src/GPIOHelper/I2CBus.hpp"
#include "../../src/common/easylogging/easylogging++.h"
#include "../../src/common/utils/commonutils.h"
#include "config.hpp"

using namespace std::chrono_literals;

//https://github.com/jrowberg/i2cdevlib/blob/master/RaspberryPi_bcm2835

INITIALIZE_EASYLOGGINGPP

void PreRollOutCallback(const char* fullPath, std::size_t s)
{
    char newPath[1024];
    char newPath2[1024];

    std::string str(fullPath);

    std::string pathNoExtension = str.substr(0, str.find_last_of('.'));

    int ver = 99;

    sprintf(newPath, "%s.log.%d", pathNoExtension.c_str(), ver);
    if(utils::FileExists(newPath)) {
        remove(newPath);
    }

    for(ver = 98; ver >= 1; ver--) {
        sprintf(newPath, "%s.log.%d", pathNoExtension.c_str(), ver);
        sprintf(newPath2, "%s.log.%d", pathNoExtension.c_str(), ver + 1);
        if(utils::FileExists(newPath)) {
            rename(newPath, newPath2);
        }
    }

    rename(fullPath, newPath);
}

int main(int argc, char** argv)
{
    std::cout << "project name: " << PROJECT_NAME << " version: " << PROJECT_VER << std::endl;

    START_EASYLOGGINGPP(argc, argv);

    if(utils::FileExists("loggerConsole.conf")) {
        // Load configuration from file
        el::Configurations conf("loggerConsole.conf");
        // Now all the loggers will use configuration from file and new loggers
        el::Loggers::setDefaultConfigurations(conf, true);
    }

    el::Helpers::setThreadName("Main");
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
    el::Helpers::installPreRollOutCallback(PreRollOutCallback);

    LOG(INFO) << "Starting Version " << PROJECT_VER;

    auto returncode = 0;

    // Todo Put I2C Device Address to Config
    auto i2cBus = new I2CBus("/dev/i2c-1");
    auto mpu = new MPU5060(i2cBus, 0x69);

    if(!mpu->InitDevice()) {
        LOG(ERROR) << "gy521 Init Failed";
    } else {
        LOG(INFO) << "gy521 Found";
    }

    int16_t ax, ay, az;
    int16_t gx, gy, gz;

    for (uint i = 0; i < 20; i++)
    {
        mpu->GetMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        //printf("  %d \t %d \t %d \t %d \t %d \t %d\r", ax, ay, az, gx, gy, gz);
        std::cout << "Motion " << ax << "\t" << ay << "\t" << az << "\t" << gx << "\t" << gy << "\t" << gz << std::endl;
        std::this_thread::sleep_for(500ms);
    }
    
    delete mpu;
    delete i2cBus;
    
    LOG(INFO) << "Last LogEntry";

    el::Helpers::uninstallPreRollOutCallback();

    return returncode;
}