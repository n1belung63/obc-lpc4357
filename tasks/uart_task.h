#pragma once

#include "../rtos_wrapper/rtos.h"

#include "../system_abstraction/board_api.h"
#include "../system_abstraction/data_storage_config.h"

#include "../application/data_storage.h"

#include "../wrappers/uart_wrapper.h"
#include "../config.h"

using namespace std::chrono_literals;

extern uint8_t buf_512[512];

template <typename TBoard>
class UartTask : public wrtos::Task<static_cast<std::size_t>(wrtos::StackDepth::minimal)> {
public:
	UartTask();
  virtual void Execute() override;
private:
	char buf_[140] = {0};				//todo: to refactor
};

template <typename TBoard>
UartTask<TBoard>::UartTask() { }

template <typename TBoard>
void UartTask<TBoard>::Execute() {
	TBoard& obc = TBoard::GetInstance();
	using UartDebug = comm::Uart<board::Uart::kDebug>;
	UartDebug& debug = UartDebug::GetInstance();

	app::TObcMagnTme tme = {0};
	int32_t res = 0;
	
	uint8_t command = 0, sd_num = 0, mpu_num = 0;
	uint16_t data_length = 0;
	
	while(1) {
		if (debug.IsByteReceived()) {
			memset(buf_, 0, sizeof(buf_));
			
			command = 0;
			debug.Read(&command, 1);
			
			switch (static_cast<debug_config::Commands>(command)) {
				case debug_config::Commands::GET_SD_STATUS: {
					break;
				}
				
				case debug_config::Commands::READ_SD: {
					using type = debug_config::READ_SD_REQUEST_t;
					debug.Read(buf_, sizeof(type));
					sd_num = ((type*)buf_)->sd_num;
					
					if ( sd_num  != static_cast<uint8_t>(board::Sd::kNum1) && sd_num  != static_cast<uint8_t>(board::Sd::kNum2) ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					res = obc.SdPageRead(static_cast<board::Sd>(sd_num), ((type*)buf_)->addr, buf_512);
					if (res != board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
						break;
					}
					
					debug.WriteByte(static_cast<char>(debug_config::Response::ACK));
					debug.Write(&buf_512[128 * ((type*)buf_)->quarter], 128);
					
					break;
				}
				
				case debug_config::Commands::WRITE_SD: {
					using type = debug_config::WRITE_SD_REQUEST_t;
					debug.Read(buf_, sizeof(type));
					sd_num = ((type*)buf_)->sd_num;

					if ( sd_num  != static_cast<uint8_t>(board::Sd::kNum1) && sd_num  != static_cast<uint8_t>(board::Sd::kNum2) ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					res = obc.SdPageRead(static_cast<board::Sd>(sd_num), ((type*)buf_)->addr, buf_512);
					if (res != board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
						break;
					}
					
					memcpy(&buf_512[((type*)buf_)->quarter * 128], ((type*)buf_)->data, 128);
					
					res = obc.SdPageWrite(static_cast<board::Sd>(sd_num), ((type*)buf_)->addr, buf_512);
					if (res != board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
						break;
					}
					
					debug.WriteByte(static_cast<char>(debug_config::Response::ACK));
	
					break;
				}

				case debug_config::Commands::ERASE_SD: {
					using type = debug_config::ERASE_SD_REQUEST_t;
					debug.Read(buf_, sizeof(type));
					sd_num = ((type*)buf_)->sd_num;
					
					if ( sd_num  != static_cast<uint8_t>(board::Sd::kNum1) && sd_num  != static_cast<uint8_t>(board::Sd::kNum2) ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					res = obc.SdRangeErase(static_cast<board::Sd>(sd_num), ((type*)buf_)->addr_start, ((type*)buf_)->addr_end);
					if (res != board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
						break;
					}
					
					debug.WriteByte(static_cast<char>(debug_config::Response::ACK));
	
					break;
				}
				
				case debug_config::Commands::BLOCK_SD: {
					using type = debug_config::BLOCK_SD_REQUEST_t;
					debug.Read(buf_, sizeof(type));
					sd_num = ((type*)buf_)->sd_num;
					
					if ( sd_num  != static_cast<uint8_t>(board::Sd::kNum1) && sd_num  != static_cast<uint8_t>(board::Sd::kNum2) ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					res = obc.SdBlock(static_cast<board::Sd>(sd_num));
					if (res != board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
						break;
					}
					
					debug.WriteByte(static_cast<char>(debug_config::Response::ACK));
	
					break;
				}
				
				case debug_config::Commands::UNBLOCK_SD: {
					using type = debug_config::UNBLOCK_SD_REQUEST_t;
					debug.Read(buf_, sizeof(type));
					sd_num = ((type*)buf_)->sd_num;
					
					if ( sd_num  != static_cast<uint8_t>(board::Sd::kNum1) && sd_num  != static_cast<uint8_t>(board::Sd::kNum2) ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					res = obc.SdUnblock(static_cast<board::Sd>(sd_num));
					if (res != board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
						break;
					}
					
					debug.WriteByte(static_cast<char>(debug_config::Response::ACK));
	
					break;
				}

				case debug_config::Commands::READ_MPU: {
					using type = debug_config::READ_MPU_REQUEST_t;
					debug.Read(buf_, sizeof(type));
					mpu_num = ((type*)buf_)->mpu_num;
					
					if ( mpu_num  != static_cast<uint8_t>(board::Magn::kNum1) && mpu_num  != static_cast<uint8_t>(board::Magn::kNum2) ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					res = obc.MagnRead(static_cast<board::Magn>(mpu_num), tme.sensors[mpu_num]);
					if (res == board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ACK));
						
						debug_config::READ_MPU_RESPONSE_t resp;
						resp.A_X = tme.sensors[mpu_num].A_X;
						resp.A_Y = tme.sensors[mpu_num].A_Y;
						resp.A_Z = tme.sensors[mpu_num].A_Z;
						resp.B_X = tme.sensors[mpu_num].B_X;
						resp.B_Y = tme.sensors[mpu_num].B_Y;
						resp.B_Z = tme.sensors[mpu_num].B_Z;
						resp.G_X = tme.sensors[mpu_num].G_X;
						resp.G_Y = tme.sensors[mpu_num].G_Y;
						resp.G_Z = tme.sensors[mpu_num].G_Z;
						resp.T 	 = tme.sensors[mpu_num].T;
						
						debug.Write(&resp, sizeof(debug_config::READ_MPU_RESPONSE_t));
					} else {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
					}
					break;						
				}
				
				default: {
					debug.WriteByte(static_cast<char>(debug_config::Response::WRONG));
					break;
				}

			}
		}
		SleepUntil(1ms);
	}
}