#include "board.h"

#include "drivers/mpu9250_driver.h"
#include "drivers/pca9554_driver.h"
#include "drivers/sdhc_driver.h"

using namespace comm;
using namespace board;

using UartDebug = comm::Uart<board::Uart::kDebug>;
using Mpu0 = sensor::Mpu9250<board::MpuAddr::A0>;
using Mpu1 = sensor::Mpu9250<board::MpuAddr::A1>;
using Pca = chip::Pca9554<board::PcaAddr::A0,0x01,0x0,0xFE>; //todo: to refactor
using Sd0 = memory::Sd<board::Sd::kNum1>;
using Sd1 = memory::Sd<board::Sd::kNum2>;

extern volatile void _delay_ms(uint32_t delay);

Board::Board() : 
	debug_(Singleton<comm::Uart<board::Uart::kDebug>>::GetInstance()),
	i2c_(Singleton<comm::I2c<board::I2c::kInt>>::GetInstance()),
	spi_(Singleton<comm::Spi<board::Spi::kSd>>::GetInstance())
{
	SystemCoreClockUpdate();
	
	RiTimerConfig((uint32_t)(SystemCoreClock / 1000));
	
	Pca& pca = Pca::GetInstance();
	pca.PinReset(static_cast<uint8_t>(board::PcaPin::kMpu0));
	pca.PinReset(static_cast<uint8_t>(board::PcaPin::kMpu1));
	pca.PinReset(static_cast<uint8_t>(board::PcaPin::kSd0));
	pca.PinReset(static_cast<uint8_t>(board::PcaPin::kSd1));
			
	_delay_ms(25);
	
	SCU_PinConfigure(0x8,7,SCU_CFG_MODE_FUNC0);	/* EN LPC3 - SD and SPIF */
	GPIO_SetDir (4, 7, GPIO_DIR_OUTPUT);
	GPIO_PinWrite(4, 7, 0);
	
	Sd0& sd0 = Sd0::GetInstance();
	Sd1& sd1 = Sd1::GetInstance();
	Mpu0& mpu0 = Mpu0::GetInstance();
	Mpu1& mpu1 = Mpu1::GetInstance();
}

void Board::RiTimerConfig(uint32_t ticks) {
	LPC_RITIMER->COMPVAL = ticks-1;	
	NVIC_SetPriority (RITIMER_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
	LPC_RITIMER->MASK = 0;
	LPC_RITIMER->COUNTER = 0;
	LPC_RITIMER->CTRL    = (1 << 3) | (1 << 2) | (1 << 1);
	
	NVIC_EnableIRQ (RITIMER_IRQn);
}