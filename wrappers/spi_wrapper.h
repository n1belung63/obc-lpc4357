#pragma once

#include "Driver_SPI.h"
#include "SSP_LPC43xx.h"

extern "C" {
#include "GPIO_LPC43xx.h"
#include "SCU_LPC43xx.h"
}

#include "singelton/singelton.h"
#include "board_settings.h"

#include <cstdint>
#include <cassert>

extern volatile void _delay_ms(uint32_t delay);
extern volatile void _delay_us(uint32_t delay);

namespace comm {
	
template <board::Spi num, uint32_t baudrate>
class Spi : public Singleton<Spi<num,baudrate>> {
	friend class Singleton<Spi<num,baudrate>>;
public:
	static constexpr int32_t  ERROR_CODE_TRANSFER_INCOMPLETE = (ARM_DRIVER_ERROR_SPECIFIC - 10);
	static constexpr int32_t  ERROR_CODE_DATA_COUNT_INCORRECT = (ARM_DRIVER_ERROR_SPECIFIC - 11);

	int32_t Write(const void *data_out, uint16_t length);
	int32_t Read(void *data_in, uint16_t length);
	int32_t Transfer(const void *data_out, void *data_in, uint16_t length);

	inline __attribute__((always_inline)) int32_t SlaveSelectEnableGpio(uint8_t port, uint8_t pin) {
		assert(drv_!=NULL);
		GPIO_PinWrite(port, pin, 0);
		return ARM_DRIVER_OK;
	}

	inline __attribute__((always_inline)) int32_t SlaveSelectDisableGpio(uint8_t port, uint8_t pin) {
		assert(drv_!=NULL);
		GPIO_PinWrite(port, pin, 1);
		return ARM_DRIVER_OK;
	}

	inline __attribute__((always_inline)) int32_t SlaveSelectEnable() {
		assert(drv_!=NULL);
		return drv_->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
	}

	inline __attribute__((always_inline)) int32_t SlaveSelectDisable() {
		assert(drv_!=NULL);
		return drv_->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
	}
private:
	static constexpr uint32_t ATTEMPTS_COUNT = 1000000;

	const Spi & operator=(const Spi &) = delete;
	constexpr Spi();
	int32_t Init();

	ARM_DRIVER_SPI* drv_;
	ARM_DRIVER_SPI* Resolve();

	inline static volatile uint32_t spi_event_;
	inline __attribute__((always_inline))static void Callback(uint32_t event) {
		spi_event_ |= event;
	}
};

template <board::Spi num, uint32_t baudrate>
ARM_DRIVER_SPI* Spi<num,baudrate>::Resolve() {
	#if (RTE_SSP0 == 1)
	if constexpr (num == board::Spi::kSd) {
		return &Driver_SPI0;
	}
	#endif
	return NULL;
}

template <board::Spi num, uint32_t baudrate>
int32_t Spi<num,baudrate>::Init() {
	assert(drv_!=NULL);
	
	int32_t res = 0;
	res = drv_->Initialize(&Callback);
	if (res != ARM_DRIVER_OK)
		return res;
	
	res = drv_->PowerControl(ARM_POWER_FULL);
	if (res != ARM_DRIVER_OK)
		return res;
	
	res = drv_->Control(ARM_SPI_MODE_MASTER |
											ARM_SPI_MSB_LSB |
											ARM_SPI_SS_MASTER_UNUSED |
											ARM_SPI_DATA_BITS(8),
											baudrate);
	
	if (res != ARM_DRIVER_OK)
		return res;
	
	return ARM_DRIVER_OK;
}

template <board::Spi num, uint32_t baudrate>
constexpr Spi<num,baudrate>::Spi() : drv_(Resolve()) {
	Spi<num,baudrate>::Init();
}

template <board::Spi num, uint32_t baudrate>
int32_t Spi<num,baudrate>::Transfer(const void *data_out, void *data_in, uint16_t length) {
	assert(drv_!=NULL);
	
	int32_t res;
	uint32_t attempts=0;
		
	res = drv_->Transfer(data_out, data_in, length);
	if (res != ARM_DRIVER_OK)
		return res;
	
	while ((spi_event_ & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U && ++attempts < ATTEMPTS_COUNT)
		__NOP();
	
	if ((spi_event_ & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U)
		return ERROR_CODE_TRANSFER_INCOMPLETE;
	 
	spi_event_ = 0U;
	
	if(drv_->GetDataCount() != length)
		return ERROR_CODE_DATA_COUNT_INCORRECT;
	
	return ARM_DRIVER_OK;
}

template <board::Spi num, uint32_t baudrate>
int32_t Spi<num,baudrate>::Write(const void *data_out, uint16_t length) {
	assert(drv_!=NULL);
	
	int32_t res;
	uint32_t attempts=0;
	uint32_t cyc = 0;
	
	res = drv_->Send(data_out, length);
	if (res != ARM_DRIVER_OK)
		return res;

	while ((spi_event_ & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U && ++attempts < ATTEMPTS_COUNT)
		__NOP();
	
	if ((spi_event_ & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U)
		return ERROR_CODE_TRANSFER_INCOMPLETE;
	 
	spi_event_ = 0U;
	
	if(drv_->GetDataCount() != length)
		return ERROR_CODE_DATA_COUNT_INCORRECT;
	
	return ARM_DRIVER_OK;
}

template <board::Spi num, uint32_t baudrate>
int32_t Spi<num,baudrate>::Read(void *data_in, uint16_t length) {
	assert(drv_!=NULL);
	
	int32_t res;
	uint32_t attempts = 0;
	
	res = drv_->Receive(data_in, length);
	if (res != ARM_DRIVER_OK)
		return res;
	
	while ((spi_event_ & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U && ++attempts < ATTEMPTS_COUNT)
		__NOP();
	
	if ((spi_event_ & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U)
		return ERROR_CODE_TRANSFER_INCOMPLETE;
	 
	spi_event_ = 0U;
	
	if(drv_->GetDataCount() != length)
		return ERROR_CODE_DATA_COUNT_INCORRECT;
	
	return ARM_DRIVER_OK;
}

}