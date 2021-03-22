#pragma once

#include "I2CDevice.hpp"

enum class pin_value;
enum class pin_direction;
class I2CBus;

/**
  * \ingroup SystemFunctions
  *
  * MPU5060
  */
class MPU5060 {
	I2CDevice* _device;
public:
	/**
	 * Create new MPU5060 Class to Control the Chip via I²C
	 * @param bus
	 *    the bus see I2CBus
	 * @param deviceAddr
	 *    The device Address
	 */
	explicit MPU5060(I2CBus* bus, unsigned char deviceAddr);
    MPU5060(const MPU5060& orig) = delete;
	MPU5060(MPU5060&& other) = delete;
	MPU5060& operator=(const MPU5060& other) = delete;
	MPU5060& operator=(MPU5060&& other) = delete;
	virtual ~MPU5060();

    bool InitDevice();
	void SetClockSource(const unsigned char source);
	void SetFullScaleGyroRange(const unsigned char range);
	void SetFullScaleAccelRange(const unsigned char range);
	void SetSleepEnabled(bool enabled);
	void GetMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);
};