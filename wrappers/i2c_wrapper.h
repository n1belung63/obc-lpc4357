#pragma once

#include "Driver_I2C.h"
#include "I2C_LPC43xx.h"

#include "../singelton.h"
#include "../board_settings.h"

#include <cstdint>
#include <cassert>

extern volatile void _delay_ms(uint32_t delay);

namespace comm {

#define I2C_ATTEMPTS_COUNT	100

#define I2C_ERROR_TRANSFER_INCOMPLETE             (ARM_DRIVER_ERROR_SPECIFIC - 10)
#define I2C_ERROR_DATA_COUNT_INCORRECT            (ARM_DRIVER_ERROR_SPECIFIC - 11)

template <board::I2c num>
class I2c : public Singleton<I2c<num>> {
	friend class Singleton<I2c<num>>;
public:
	int32_t Clear();
	int32_t Write(const uint8_t addr, uint8_t* data, uint16_t length);
	int32_t Read(const uint8_t addr, uint8_t* data, uint16_t length);
private:
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

template <board::I2c num>
ARM_DRIVER_I2C* I2c<num>::Resolve() {
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

template <board::I2c num>
I2c<num>::I2c() : drv_(Resolve()) {
	Init();
}

template <board::I2c num>
int32_t I2c<num>::Init() {
	assert(drv_!=NULL);
	
	int32_t res = 0;

	res = drv_->Initialize(&Callback);
	if (res != ARM_DRIVER_OK) 
		return res;
	
	res = drv_->PowerControl(ARM_POWER_FULL);
	if (res != ARM_DRIVER_OK)
		return res;
	
	res = drv_->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
	if (res != ARM_DRIVER_OK)
		return res;
		
	return ARM_DRIVER_OK;
}

template <board::I2c num>
int32_t I2c<num>::Clear() {
	assert(drv_!=NULL);
	
	int32_t res = 0;
	
	res = drv_->Control(ARM_I2C_BUS_CLEAR,0);
	if (res != ARM_DRIVER_OK)
		return res;
	
	return ARM_DRIVER_OK;
}

template <board::I2c num>
int32_t I2c<num>::Write(const uint8_t addr, uint8_t* data, uint16_t length) {
	assert(drv_!=NULL);
	
	int32_t res = 0;
	int32_t attempts = 0;
	
	i2c_event_ = 0U;

	res = drv_->MasterTransmit(addr, data, (uint32_t)length, false);
	if (res != ARM_DRIVER_OK)
		return res;
			
	while ((i2c_event_ & ARM_I2C_EVENT_TRANSFER_DONE) == 0U && ++attempts < I2C_ATTEMPTS_COUNT)
		_delay_ms(1);
		
	if ((i2c_event_ & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U)
		return I2C_ERROR_TRANSFER_INCOMPLETE;
 
	i2c_event_ = 0U;
	
	if(drv_->GetDataCount() != length)
		return I2C_ERROR_DATA_COUNT_INCORRECT;
	
	return ARM_DRIVER_OK;
}

template <board::I2c num>
int32_t I2c<num>::Read(const uint8_t addr, uint8_t* data, uint16_t length) {
	assert(drv_!=NULL);
	
	int32_t res = 0;
	int32_t attempts = 0;
	
	i2c_event_ = 0U;
	
	res = drv_->Control(ARM_I2C_BUS_CLEAR, 0);
	if (res != ARM_DRIVER_OK)
		return res;
	
	res = drv_->MasterReceive(addr, (uint8_t *)data, (uint32_t)length, false);
	if (res != ARM_DRIVER_OK)
		return res;
	
	while ((i2c_event_ & ARM_I2C_EVENT_TRANSFER_DONE) == 0U && ++attempts < I2C_ATTEMPTS_COUNT)
		_delay_ms(1);
	
	if ((i2c_event_ & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) != 0U)
		return I2C_ERROR_TRANSFER_INCOMPLETE;
	
	i2c_event_ = 0U;
	
	if(drv_->GetDataCount() != length)
		return I2C_ERROR_DATA_COUNT_INCORRECT;
	
	return ARM_DRIVER_OK;
}

}
