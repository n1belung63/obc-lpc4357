#pragma once

#include "Driver_USART.h"
#include "USART_LPC43xx.h"

#include "../singelton/singelton.h"
#include "../board_settings.h"

#include <cstdint>
#include <cassert>
#include <cstdio>
#include <cstring>

extern volatile void _delay_ms(uint32_t delay);

namespace comm {

class UartBuffer {
	static constexpr int buf_size = 140;
public:
	inline void set_to_buf(const volatile unsigned int& in) {
		sprintf(buf_, "%u", in);
	}
	inline void set_to_buf(const unsigned int& in) {
		sprintf(buf_, "%u", in);
	}
	inline void set_to_buf(const volatile int& in) {
		sprintf(buf_, "%d", in);
	}
	inline void set_to_buf(const int& in) {
		sprintf(buf_, "%d", in);
	}
	inline void set_to_buf(const char *in) {
		sprintf(buf_, "%s", in);
	}
	
	inline int StrLen() {
		return std::strlen(buf_);
	}

	inline char* GetAddress() noexcept {
		return buf_;
	}
	
private:
	char buf_[buf_size] = {0};
};

template <board::Uart num>
class Uart : public Singleton<Uart<num>> {
	friend class Singleton<Uart<num>>;
public:
	int32_t Write(const void *data, uint16_t length);
	int32_t Read(void *data, uint16_t length);
	bool IsByteReceived(void);
	int32_t WriteByte(const char inp);

	UartBuffer buf;
private:
	static constexpr uint32_t BAUDRATE = 115200;
	static constexpr uint32_t ATTEMPTS_COUNT = 10000;

	const Uart & operator=(const Uart &) = delete;
	Uart();
	int32_t Init();

	ARM_DRIVER_USART* drv_;
	ARM_DRIVER_USART* Resolve();
	LPC_USARTn_Type* ResolveType();

	inline static uint32_t uart_event_ = 0U;
	inline static void Callback(uint32_t event) {
		uart_event_ |= event;
	}
};

template<typename Type, board::Uart num>
Uart<num> & operator << (Uart<num> &out, const Type& in) {
	out.buf.set_to_buf(in);
	out.Write(static_cast<void*>(out.buf.GetAddress()), out.buf.StrLen());		
		
  return out;
}

template <board::Uart num>
ARM_DRIVER_USART* Uart<num>::Resolve() {
	#if (RTE_USART3 == 1)
	if constexpr (num == board::Uart::kDebug) {
		return &Driver_USART3;
	}
	#endif
	return NULL;
}

template <board::Uart num>
LPC_USARTn_Type* Uart<num>::ResolveType() {
	#if (RTE_USART3 == 1)
	if constexpr (num == board::Uart::kDebug) {
		return LPC_USART3;
	}
	#endif
	return NULL;
}

template <board::Uart num>
int32_t Uart<num>::Init() {
	assert(drv_!=NULL);
	
	int32_t res = 0;

	res = drv_->Initialize(&Callback);
	if (res != ARM_DRIVER_OK)
		return res;
	
	res = drv_->PowerControl(ARM_POWER_FULL);
	if (res != ARM_DRIVER_OK)
		return res;

	res = drv_->Control(	ARM_USART_MODE_ASYNCHRONOUS |
													ARM_USART_DATA_BITS_8 |
													ARM_USART_PARITY_NONE |
													ARM_USART_STOP_BITS_1 |
													ARM_USART_FLOW_CONTROL_NONE,
													BAUDRATE);
	if (res != ARM_DRIVER_OK)
		return res;
	
	res = drv_->Control(ARM_USART_CONTROL_TX, 1);
	if (res != ARM_DRIVER_OK)
		return res;
	
	res = drv_->Control(ARM_USART_CONTROL_RX, 1);
	if (res != ARM_DRIVER_OK)
		return res;
	
	return ARM_DRIVER_OK;
}

template <board::Uart num>
Uart<num>::Uart() : drv_(Resolve()) {
	Uart<num>::Init();
}

template <board::Uart num>
int32_t Uart<num>::Write(const void *data_out, uint16_t length) {
	assert(drv_!=NULL);
	
	uint32_t attempts=0;
	
	LPC_USARTn_Type* USART_periph = ResolveType();	
	uint8_t* data = (uint8_t*)data_out;

	for (uint16_t i = 0; i < length; i++) {
		attempts = 0;
		while (!(USART_periph->LSR & USART_LSR_TEMT)) {
			if (++attempts > ATTEMPTS_COUNT)
				return ARM_DRIVER_ERROR_TIMEOUT;
		}
		USART_periph->THR = data[i];
	}
	
	return ARM_DRIVER_OK;
}

template <board::Uart num>
int32_t Uart<num>::WriteByte(const char inp) {	
	return Write((char[1]){inp}, 1);
}

template <board::Uart num>
int32_t Uart<num>::Read(void *data_in, uint16_t length) {
	assert(drv_!=NULL);
	
	uint32_t attempts=0;

	LPC_USARTn_Type* USART_periph = ResolveType();	
	uint8_t* data = (uint8_t*)data_in;

	for (uint16_t i = 0; i < length; ++i) {
		attempts = 0;
		while (!(USART_periph->LSR & USART_LSR_RDR))
		{ 
			if (++attempts > ATTEMPTS_COUNT)
				return ARM_DRIVER_ERROR_TIMEOUT;
		}
		data[i] = USART_periph->RBR;
	}
	
	return ARM_DRIVER_OK;
}

template <board::Uart num>
bool Uart<num>::IsByteReceived(void) {
	assert(drv_!=NULL);
	
	LPC_USARTn_Type* USART_periph = ResolveType();
	
	if ((USART_periph->LSR & USART_LSR_RDR))
		return true;
	
	return false;
}

}
