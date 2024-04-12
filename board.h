#pragma once

#include "LPC43xx.h"
#include "system_LPC43xx.h"

#include "board_settings.h"

extern "C" {
#include "GPIO_LPC43xx.h"
#include "SCU_LPC43xx.h"
}

#include "wrappers/uart_wrapper.h"
#include "wrappers/i2c_wrapper.h"
#include "wrappers/spi_wrapper.h"
#include "singelton.h"

namespace board {
struct Board : public Singleton<Board> {
	friend class Singleton<Board>;
	friend class Singleton<comm::Uart<board::Uart::kDebug>>;
	friend class Singleton<comm::I2c<board::I2c::kInt>>;
	friend class Singleton<comm::Spi<board::Spi::kSd>>;
private:
	comm::Uart<board::Uart::kDebug>& debug_;
	comm::I2c<board::I2c::kInt>& i2c_;
	comm::Spi<board::Spi::kSd>& spi_;

	Board();
	void RiTimerConfig(uint32_t);
};

}