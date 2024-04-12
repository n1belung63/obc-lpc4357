#include "uart_task.h"

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

using UartDebug = comm::Uart<board::Uart::kDebug>;
using Mpu0 = sensor::Mpu9250<board::MpuAddr::A0>;
using Mpu1 = sensor::Mpu9250<board::MpuAddr::A1>;
using Sd0 = memory::Sd<board::Sd::kNum1>;
using Sd1 = memory::Sd<board::Sd::kNum2>;

uint8_t sd_buf[512] = {0};	//todo: to refactor
char buf[140] = {0};				//todo: to refactor

void UartTask::Execute() {
	UartDebug& debug = UartDebug::GetInstance();
	Mpu0& mpu0 = Mpu0::GetInstance();
	Mpu1& mpu1 = Mpu1::GetInstance();
	Sd0& sd0 = Sd0::GetInstance();
	Sd1& sd1 = Sd1::GetInstance();
	
	sensor::MPU9250_DATA_t mpu_data = {0};
	int32_t res = 0;
	
	uint8_t command = 0;
	uint16_t data_length = 0;
	
	while(1) {
		if (debug.IsByteReceived()) {
			memset(buf, 0, sizeof(buf));
			
			command = 0;
			debug.Read(&command, 1);
			
			switch (static_cast<config::Commands>(command)) {
				case config::Commands::GET_SD_STATUS: {
					break;
				}
				
				case config::Commands::READ_SD: {
					using type = config::READ_SD_REQUEST_t;
					data_length = sizeof(type);
					debug.Read(buf, data_length);
					
					if ( ((type*)buf)->sd_num == static_cast<uint8_t>(board::Sd::kNum1) ) {
						
						res = sd0.ReadSingleBlock(((type*)buf)->addr, sd_buf);
						if (res != sd0.SD_DRIVER_OK) {
							debug.WriteByte(static_cast<char>(config::Response::NACK));
							debug.Write(&res, sizeof(res));
							break;
						}					
						debug.WriteByte(static_cast<char>(config::Response::ACK));
						debug.Write(&sd_buf[128 * ((type*)buf)->quarter], 128);
						break;
						
					} else if ( ((type*)buf)->sd_num == static_cast<uint8_t>(board::Sd::kNum2) ) {
						
						res = sd1.ReadSingleBlock(((type*)buf)->addr, sd_buf);
						if (res != sd1.SD_DRIVER_OK) {
							debug.WriteByte(static_cast<char>(config::Response::NACK));
							debug.Write(&res, sizeof(res));
							break;
						}
						debug.WriteByte(static_cast<char>(config::Response::ACK));
						debug.Write(&sd_buf[128 * ((type*)buf)->quarter], 128);
						break;
						
					} else {
						debug.WriteByte(static_cast<char>(config::Response::ERR));
						break;
					}
					
					break;
				}
				
				case config::Commands::WRITE_SD: {
					using type = config::WRITE_SD_REQUEST_t;
					data_length = sizeof(type);
					res = debug.Read(buf, data_length);						
					
					if ( ((type*)buf)->sd_num  == static_cast<uint8_t>(board::Sd::kNum1) ) {
						
						res = sd0.ReadSingleBlock(((type*)buf)->addr, sd_buf);
						if (res != sd0.SD_DRIVER_OK) {
							debug.WriteByte(static_cast<char>(config::Response::NACK));
							debug.Write(&res, sizeof(res));
							break;
						}
						memcpy(&sd_buf[((type*)buf)->quarter * 128], ((type*)buf)->data, 128);
						res = sd0.WriteSingleBlock(((type*)buf)->addr, sd_buf);
						if (res != sd0.SD_DRIVER_OK) {
							debug.WriteByte(static_cast<char>(config::Response::NACK));
							debug.Write(&res, sizeof(res));
							break;
						}
						debug.WriteByte(static_cast<char>(config::Response::ACK));
						break;
						
					} else if ( ((type*)buf)->sd_num == static_cast<uint8_t>(board::Sd::kNum2) ) {
						
						res = sd1.ReadSingleBlock(((type*)buf)->addr, sd_buf);
						if (res != sd1.SD_DRIVER_OK) {
							debug.WriteByte(static_cast<char>(config::Response::NACK));
							debug.Write(&res, sizeof(res));
							break;
						}
						memcpy(&sd_buf[((type*)buf)->quarter * 128], ((type*)buf)->data, 128);
						res = sd1.WriteSingleBlock(((type*)buf)->addr, sd_buf);
						if (res != sd1.SD_DRIVER_OK) {
							debug.WriteByte(static_cast<char>(config::Response::NACK));
							debug.Write(&res, sizeof(res));
							break;
						}
						debug.WriteByte(static_cast<char>(config::Response::ACK));
						break;
						
					} else {
						debug.WriteByte(static_cast<char>(config::Response::ERR));
						break;
					}
					
					break;
				}

				case config::Commands::ERASE_SD: {
					break;
				}

				case config::Commands::READ_MPU: {
					data_length = sizeof(config::READ_MPU_REQUEST_t);
					debug.Read(buf, data_length);
					
					if (buf[0] == 0) {
						res = mpu0.Read(&mpu_data);						
					} else if (buf[0] == 1) {
						res = mpu1.Read(&mpu_data);
					} else {
						debug.WriteByte(static_cast<char>(config::Response::ERR));
						break;
					}
					
					if (res == Mpu0::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(config::Response::ACK));
						
						config::READ_MPU_RESPONSE_t resp;
						resp.A_X = mpu_data.A_X;
						resp.A_Y = mpu_data.A_Y;
						resp.A_Z = mpu_data.A_Z;
						resp.B_X = mpu_data.B_X;
						resp.B_Y = mpu_data.B_Y;
						resp.B_Z = mpu_data.B_Z;
						resp.G_X = mpu_data.G_X;
						resp.G_Y = mpu_data.G_Y;
						resp.G_Z = mpu_data.G_Z;
						resp.T 	 = mpu_data.T;
						
						debug.Write(&resp, sizeof(config::READ_MPU_RESPONSE_t));
					} else {
						debug.WriteByte(static_cast<char>(config::Response::NACK));
						debug.Write(&res, sizeof(res));
					}
					break;						
				}
				
				default: {
					debug.WriteByte(static_cast<char>(config::Response::WRONG));
					break;
				}

			}
		}
		SleepUntil(1ms);
	}
}