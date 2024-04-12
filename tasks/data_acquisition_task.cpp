#include "data_acquisition_task.h"

#include "../board.h"

#include "../rtos_wrapper/rtos.h"

#include "../wrappers/uart_wrapper.h"
#include "../wrappers/i2c_wrapper.h"
#include "../wrappers/spi_wrapper.h"

#include "../drivers/mpu9250_driver.h"
#include "../drivers/sdhc_driver.h"

#include "../config.h"

using namespace wrtos;
using namespace board;

//using UartDebug = comm::Uart<board::Uart::kDebug>;
using Mpu0 = sensor::Mpu9250<board::MpuAddr::A0>;
using Mpu1 = sensor::Mpu9250<board::MpuAddr::A1>;
using Sd0 = memory::Sd<board::Sd::kNum1>;
using Sd1 = memory::Sd<board::Sd::kNum2>;

void DataAcquisitionTask::Execute() {
//	UartDebug& debug = UartDebug::GetInstance();
	Sd0& sd0 = Sd0::GetInstance();
	Sd1& sd1 = Sd1::GetInstance();
	Mpu0& mpu0 = Mpu0::GetInstance();
	Mpu1& mpu1 = Mpu1::GetInstance();
	
	sensor::MPU9250_DATA_t mpu_data = {0};
	int32_t res = 0;

	while(true) {	
			res = mpu0.Read(&mpu_data);
			SleepUntil(250ms);
	}
}