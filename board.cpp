#include "board.h"

#include "drivers/mpu9250_driver.h"
#include "drivers/pca9554_driver.h"
#include "drivers/sdhc_driver.h"

using namespace comm;
using namespace board;

using UartDebug = comm::Uart<board::Uart::kDebug, board::UART_DEBUG_BAUDRATE>;
using IntI2C = comm::I2c<board::I2c::kInt, board::I2C_INT_SPEED>;
using SpiSd = comm::Spi<board::Spi::kSd, board::SPI_SD_BAUDRATE>;

using Mpu0 = sensor::Mpu9250<board::MpuAddr::A0>;
using Mpu1 = sensor::Mpu9250<board::MpuAddr::A1>;
using Pca = chip::Pca9554<board::PcaAddr::A0,board::PCA_DIR,board::PCA_POL,board::PCA_STATE>;
using Sd0 = memory::Sd<board::Sd::kNum1>;
using Sd1 = memory::Sd<board::Sd::kNum2>;

extern volatile void _delay_ms(uint32_t delay);

Board::Board() {
	SystemCoreClockUpdate();
	
	/* SD1 and SD2 Slave Select setup */
	SCU_PinConfigure(SD1_PORT, SD1_PIN, SCU_CFG_MODE_FUNC4);
	SCU_PinConfigure(SD2_PORT, SD2_PIN, SCU_CFG_MODE_FUNC4);
	GPIO_PortClock(1);
	GPIO_SetDir(SD1_GPIO_PORT, SD1_GPIO_PIN, GPIO_DIR_OUTPUT);
	GPIO_PinWrite(SD1_GPIO_PORT, SD1_GPIO_PIN, 1);
	GPIO_SetDir(SD2_GPIO_PORT, SD2_GPIO_PIN, GPIO_DIR_OUTPUT);
	GPIO_PinWrite(SD2_GPIO_PORT, SD2_GPIO_PIN, 1);
	SCU_PinConfigure(RTE_SSP0_SCK_PORT, RTE_SSP0_SCK_BIT, SCU_CFG_MODE_FUNC1);
	
	UartDebug& debug = UartDebug::GetInstance();
	IntI2C& i2c = IntI2C::GetInstance();
	SpiSd& spi = SpiSd::GetInstance();
	
	Pca& pca = Pca::GetInstance();
	
	if (pca.GetLastErrorCode() != pca.ERROR_CODE_OK) {
		__NOP(); //todo: perform error
	}
		
	pca.PinReset(static_cast<uint8_t>(board::PcaPin::kMpu0));
	pca.PinReset(static_cast<uint8_t>(board::PcaPin::kMpu1));
	pca.PinReset(static_cast<uint8_t>(board::PcaPin::kSd0));
	pca.PinReset(static_cast<uint8_t>(board::PcaPin::kSd1));
			
	_delay_ms(25);
	
	/* EN LPC3 - SD and SPIF */
	SCU_PinConfigure(LPC3_PORT, LPC3_PIN, SCU_CFG_MODE_FUNC0);
	GPIO_SetDir(LPC3_GPIO_PORT, LPC3_GPIO_PIN, GPIO_DIR_OUTPUT);
	GPIO_PinWrite(LPC3_GPIO_PORT, LPC3_GPIO_PIN, 0);
	
	Sd0& sd0 = Sd0::GetInstance();
	Sd1& sd1 = Sd1::GetInstance();
	Mpu0& mpu0 = Mpu0::GetInstance();
	Mpu1& mpu1 = Mpu1::GetInstance();
		
	if (sd0.GetLastErrorCode() != sd0.ERROR_CODE_OK)
		status_pool_.sd[static_cast<uint8_t>(Sd::kNum1)] = Status::kFailed;
	if (sd1.GetLastErrorCode() != sd1.ERROR_CODE_OK)
		status_pool_.sd[static_cast<uint8_t>(Sd::kNum2)] = Status::kFailed;
	if (mpu0.GetLastErrorCode() != mpu0.ERROR_CODE_OK)
		status_pool_.mpu[static_cast<uint8_t>(Magn::kNum1)] = Status::kFailed;
	if (mpu1.GetLastErrorCode() != mpu1.ERROR_CODE_OK)
		status_pool_.mpu[static_cast<uint8_t>(Magn::kNum2)] = Status::kFailed;
}

int32_t Board::SdPageWrite(Sd num, uint32_t page_addr, uint8_t page[board::SD_PAGE_SIZE]) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);
	
	switch(num) {
		case Sd::kNum1: {
			if (status_pool_.sd[static_cast<uint8_t>(Sd::kNum1)] == Status::kFailed) {
				return ERROR_CODE_NOT_INITED;
			}		
			Sd0& sd0 = Sd0::GetInstance();
			return sd0.WriteSingleBlock(page_addr, page);
		}
			
		case Sd::kNum2: {
			if (status_pool_.sd[static_cast<uint8_t>(Sd::kNum2)]  == Status::kFailed) {
				return ERROR_CODE_NOT_INITED;
			}	
			Sd1& sd1 = Sd1::GetInstance();
			return sd1.WriteSingleBlock(page_addr, page);
		}
	}
}

int32_t Board::SdPageRead(Sd num, uint32_t page_addr, uint8_t page[board::SD_PAGE_SIZE]) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);
	
	switch(num) {
		case Sd::kNum1: {
			if (status_pool_.sd[static_cast<uint8_t>(Sd::kNum1)] == Status::kFailed) {
				return ERROR_CODE_NOT_INITED;
			}					
			Sd0& sd0 = Sd0::GetInstance();
			return sd0.ReadSingleBlock(page_addr, page);
		}
			
		case Sd::kNum2: {
			if (status_pool_.sd[static_cast<uint8_t>(Sd::kNum2)] == Status::kFailed) {
				return ERROR_CODE_NOT_INITED;
			}	
			Sd1& sd1 = Sd1::GetInstance();
			return sd1.ReadSingleBlock(page_addr, page);
		}
	}
}

int32_t Board::SdBlock(Sd num) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);
	
	if (GetSdStatus(num) == Status::kWorked) {
		status_pool_.sd[static_cast<uint8_t>(num)] = Status::kBlocked;
	}
	return ERROR_CODE_OK;
}

int32_t Board::SdUnblock(Sd num) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);
	
	if (GetSdStatus(num) == Status::kBlocked) {
		status_pool_.sd[static_cast<uint8_t>(num)] = Status::kWorked;
	}
	return ERROR_CODE_OK;
}

int32_t Board::SdRangeErase(Sd num, uint32_t start_addr, uint32_t end_addr) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);
	
	switch(num) {
		case Sd::kNum1: {
			if (status_pool_.sd[static_cast<uint8_t>(Sd::kNum1)] == Status::kFailed) {
				return ERROR_CODE_NOT_INITED;
			}					
			Sd0& sd0 = Sd0::GetInstance();
			return sd0.Erase(start_addr, end_addr);
		}
			
		case Sd::kNum2: {
			if (status_pool_.sd[static_cast<uint8_t>(Sd::kNum2)] == Status::kFailed) {
				return ERROR_CODE_NOT_INITED;
			}	
			Sd1& sd1 = Sd1::GetInstance();
			return sd1.Erase(start_addr, end_addr);
		}
	}
}

int32_t Board::MagnRead(Magn num, MagnData& data) {
	assert(num == Magn::kNum1 || num == Magn::kNum2);
	
	switch(num) {
		case Magn::kNum1: {
			if (status_pool_.mpu[static_cast<uint8_t>(Magn::kNum1)] == Status::kFailed) {
				return ERROR_CODE_NOT_INITED;
			}
			
			Mpu0& mpu0 = Mpu0::GetInstance();
			sensor::MPU9250_DATA_t mpu_data = {0};
			int32_t res = mpu0.Read(&mpu_data);
			if (res != ERROR_CODE_OK) {
				status_pool_.mpu[static_cast<uint8_t>(Magn::kNum1)] = Status::kFailed;
				return res;
			}
			
			data.A_X = mpu_data.A_X;
			data.A_Y = mpu_data.A_Y;
			data.A_Z = mpu_data.A_Z;
			data.B_X = mpu_data.B_X;
			data.B_Y = mpu_data.B_Y;
			data.B_Z = mpu_data.B_Z;
			data.G_X = mpu_data.G_X;
			data.G_Y = mpu_data.G_Y;
			data.G_Z = mpu_data.G_Z;
			data.T 	 = mpu_data.T;	
			
			return res;
		}
			
		case Magn::kNum2: {
			if (status_pool_.mpu[static_cast<uint8_t>(Magn::kNum2)] == Status::kFailed) {
				return ERROR_CODE_NOT_INITED;
			}
			
			Mpu1& mpu1 = Mpu1::GetInstance();
			sensor::MPU9250_DATA_t mpu_data = {0};
			int32_t res = mpu1.Read(&mpu_data);
			if (res != ERROR_CODE_OK) {
				status_pool_.mpu[static_cast<uint8_t>(Magn::kNum1)] = Status::kFailed;
				return res;
			}
			
			data.A_X = mpu_data.A_X;
			data.A_Y = mpu_data.A_Y;
			data.A_Z = mpu_data.A_Z;
			data.B_X = mpu_data.B_X;
			data.B_Y = mpu_data.B_Y;
			data.B_Z = mpu_data.B_Z;
			data.G_X = mpu_data.G_X;
			data.G_Y = mpu_data.G_Y;
			data.G_Z = mpu_data.G_Z;
			data.T 	 = mpu_data.T;	
			
			return res;
		}
	}
}

Status Board::GetMagnStatus(Magn num) {
	assert(num == Magn::kNum1 || num == Magn::kNum2);
	return status_pool_.mpu[static_cast<uint8_t>(num)];
}
Status Board::GetSdStatus(Sd num) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);
	return status_pool_.sd[static_cast<uint8_t>(num)];
}
