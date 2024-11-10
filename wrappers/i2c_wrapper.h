#pragma once

#include "Driver_I2C.h"
#include "I2C_LPC43xx.h"

#include "singelton/singelton.h"
#include "board_settings.h"

#include <cstdint>
#include <cassert>

extern volatile void _delay_ms(uint32_t delay);

namespace comm {

template <board::I2c num, uint32_t baudrate>
class I2c : public Singleton<I2c<num, baudrate>> {
	friend class Singleton<I2c<num, baudrate>>;
public:
	static constexpr int32_t ERROR_CODE_OK = 0;
	static constexpr int32_t  ERROR_CODE_TRANSFER_INCOMPLETE = (ARM_DRIVER_ERROR_SPECIFIC - 10);
	static constexpr int32_t  ERROR_CODE_DATA_COUNT_INCOMPLETE = (ARM_DRIVER_ERROR_SPECIFIC - 11);

	int32_t Clear();
	int32_t Write(const uint8_t addr, uint8_t* data, uint16_t length);
	int32_t Read(const uint8_t addr, uint8_t* data, uint16_t length);
private:
	static constexpr uint32_t ATTEMPTS_COUNT = 1000000;

	static constexpr uint32_t BUS_SPEED_STANDARD = 100000;
	static constexpr uint32_t BUS_SPEED_FAST = 400000;
	static constexpr uint32_t BUS_SPEED_FAST_PLUS = 1000000;
	static constexpr uint32_t BUS_SPEED_HIGH = 3400000;

	const I2c & operator=(const I2c &) = delete;
	I2c();
	int32_t Init();

	ARM_DRIVER_I2C* drv_;
	ARM_DRIVER_I2C* Resolve();

	inline static uint32_t i2c_event_;
	inline static void Callback(uint32_t event) {
		i2c_event_ |= event;
	}
};

template <board::I2c num, uint32_t baudrate>
ARM_DRIVER_I2C* I2c<num, baudrate>::Resolve() {
	#if (RTE_I2C0 == 1)
	if constexpr (num == board::I2c::kInt) {
		return &Driver_I2C0;
	}
	#elif (RTE_I2C1 == 1)
	if constexpr (num == board::I2c::kExt) {
		return &Driver_I2C1;
	}
	#endif

	return NULL;
}

template <board::I2c num, uint32_t baudrate>
I2c<num, baudrate>::I2c() : drv_(Resolve()) {
	Init();
}

template <board::I2c num, uint32_t baudrate>
int32_t I2c<num, baudrate>::Init() {
	assert(drv_!=NULL);
	static_assert(baudrate == BUS_SPEED_STANDARD | baudrate == BUS_SPEED_FAST | baudrate == BUS_SPEED_FAST_PLUS | baudrate == BUS_SPEED_HIGH);

	int32_t res = 0;

	res = drv_->Initialize(&Callback);
	if (res != ARM_DRIVER_OK) 
		return res;
	
	res = drv_->PowerControl(ARM_POWER_FULL);
	if (res != ARM_DRIVER_OK)
		return res;
	
	uint32_t speed_mode;
	switch(baudrate) {
		case BUS_SPEED_STANDARD: 
			speed_mode = ARM_I2C_BUS_SPEED_STANDARD;
			break;
		case BUS_SPEED_FAST: 
			speed_mode = ARM_I2C_BUS_SPEED_FAST; 
			break;
		case BUS_SPEED_FAST_PLUS: 
			speed_mode = ARM_I2C_BUS_SPEED_FAST_PLUS; 
			break;
		case BUS_SPEED_HIGH: 
			speed_mode = ARM_I2C_BUS_SPEED_HIGH; 
			break;
	}
	
	res = drv_->Control(ARM_I2C_BUS_SPEED, speed_mode);
	if (res != ARM_DRIVER_OK)
		return res;
		
	return ARM_DRIVER_OK;
}

template <board::I2c num, uint32_t baudrate>
int32_t I2c<num, baudrate>::Clear() {
	assert(drv_!=NULL);
	
	int32_t res = 0;
	
	res = drv_->Control(ARM_I2C_BUS_CLEAR,0);
	if (res != ARM_DRIVER_OK)
		return res;
	
	return ARM_DRIVER_OK;
}

template <board::I2c num, uint32_t baudrate>
int32_t I2c<num, baudrate>::Write(const uint8_t addr, uint8_t* data, uint16_t length) {
	assert(drv_!=NULL);
	
	int32_t res = 0;
	int32_t attempts = 0;
	
	i2c_event_ = 0U;

	res = drv_->MasterTransmit(addr, data, (uint32_t)length, false);
	if (res != ARM_DRIVER_OK)
		return res;
			
	while ((i2c_event_ & ARM_I2C_EVENT_TRANSFER_DONE) == 0U && ++attempts < ATTEMPTS_COUNT)
		__NOP();
		
	if ((i2c_event_ & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U)
		return ERROR_CODE_TRANSFER_INCOMPLETE;
 
	i2c_event_ = 0U;
	
	if(drv_->GetDataCount() != length)
		return ERROR_CODE_DATA_COUNT_INCOMPLETE;
	
	return ARM_DRIVER_OK;
}

template <board::I2c num, uint32_t baudrate>
int32_t I2c<num, baudrate>::Read(const uint8_t addr, uint8_t* data, uint16_t length) {
	assert(drv_!=NULL);
	
	int32_t res = 0;
	int32_t attempts = 0;
	
	i2c_event_ = 0U;
	
	res = drv_->MasterReceive(addr, (uint8_t *)data, (uint32_t)length, false);
	if (res != ARM_DRIVER_OK)
		return res;
	
	while ((i2c_event_ & ARM_I2C_EVENT_TRANSFER_DONE) == 0U && ++attempts < ATTEMPTS_COUNT)
		__NOP();
	
	if ((i2c_event_ & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U)
		return ERROR_CODE_TRANSFER_INCOMPLETE;
	
	i2c_event_ = 0U;
	
	if(drv_->GetDataCount() != length)
		return ERROR_CODE_DATA_COUNT_INCOMPLETE;
	
	return ARM_DRIVER_OK;
}

}
