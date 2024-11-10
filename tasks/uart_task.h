#pragma once

#include "rtos_wrapper/rtos.h"

#include "system_abstraction/board_api.h"
#include "system_abstraction/data_storage_config.h"

#include "application/data_storage.h"

#include "wrappers/uart_wrapper.h"
#include "config.h"

#include "pool_allocator_port.h"

#include "singelton/singelton.h"

using namespace std::chrono_literals;

template<typename TPoolAllocatorPort>
using DefaultAllocator = allocator::PoolAllocator<allocator::DefaultBlockSize,allocator::DefaultBlockCount, TPoolAllocatorPort>;

template <typename TBoard>
class UartTask : public wrtos::Task<static_cast<std::size_t>(wrtos::StackDepth::big)> {
public:
	UartTask();
  virtual void Execute() override;
private:
	DefaultAllocator<allocator::PoolAllocatorPort>& allocator_;
};

template <typename TBoard>
UartTask<TBoard>::UartTask()
	: allocator_(Singleton<DefaultAllocator<allocator::PoolAllocatorPort>>::GetInstance())
	{ }

template <typename TBoard>
void UartTask<TBoard>::Execute() {
	using namespace allocator;
	using Sector = app::DataStorageConfig::Sector;
	
	TBoard& obc = TBoard::GetInstance();
	app::DataStorage<TBoard,PoolAllocatorPort,app::DataStorageConfig>& data_storage = 
			app::DataStorage<TBoard,PoolAllocatorPort,app::DataStorageConfig>::GetInstance();

	using UartDebug = comm::Uart<board::Uart::kDebug, board::UART_DEBUG_BAUDRATE>;
	UartDebug& debug = UartDebug::GetInstance();

	static app::TObcMagnTme tme = {0};
	static debug_config::READ_MPU_RESPONSE_t read_mpu_resp;
	static debug_config::READ_TME_RESPONSE_t read_tme_resp;
	static debug_config::READ_TME_BUNCH_RESPONSE_t read_tme_bunch_resp;
	static int32_t res = 0;
	static uint8_t command = 0;
	static uint8_t sd_num = 0;
	static uint8_t mpu_num = 0;
	static uint8_t sector_num = 0;
	static uint16_t data_length = 0;
	static uint8_t* buf_140;
	static uint8_t* buf_512;
	
	static const uint8_t sectors_count = app::DataStorageConfig::GetSectorsCount();
	static app::TDataStorageSectorPars sector_pars;
	
	static constexpr uint16_t BUF_140_SIZE = 140;
	static constexpr uint16_t BUF_512_SIZE = 512;

	while(true) {
		if (debug.IsByteReceived()) {
			buf_140 = static_cast<uint8_t*>(allocator_.allocate(BUF_140_SIZE));
			buf_512 = static_cast<uint8_t*>(allocator_.allocate(BUF_512_SIZE));
			
			memset(buf_140, 0, BUF_140_SIZE);
			
			command = 0;
			debug.Read(&command, 1);
			
			switch (static_cast<debug_config::Commands>(command)) {
				case debug_config::Commands::GET_SD_STATUS: {
					break;
				}
				
				case debug_config::Commands::READ_SD: {
					using type = debug_config::READ_SD_REQUEST_t;
					debug.Read(buf_140, sizeof(type));
					sd_num = ((type*)buf_140)->sd_num;
					
					if ( sd_num  != static_cast<uint8_t>(board::Sd::kNum1) && sd_num  != static_cast<uint8_t>(board::Sd::kNum2) ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					res = obc.SdPageRead(static_cast<board::Sd>(sd_num), ((type*)buf_140)->addr, buf_512);
					if (res != board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
						break;
					}
					
					debug.WriteByte(static_cast<char>(debug_config::Response::ACK));
					debug.Write(&buf_512[debug_config::SD_PAGE_QUARTER_LENGTH * ((type*)buf_140)->quarter], debug_config::SD_PAGE_QUARTER_LENGTH);
					
					break;
				}
				
				case debug_config::Commands::WRITE_SD: {
					using type = debug_config::WRITE_SD_REQUEST_t;
					debug.Read(buf_140, sizeof(type));
					sd_num = ((type*)buf_140)->sd_num;

					if ( sd_num  != static_cast<uint8_t>(board::Sd::kNum1) && sd_num  != static_cast<uint8_t>(board::Sd::kNum2) ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					res = obc.SdPageRead(static_cast<board::Sd>(sd_num), ((type*)buf_140)->addr, buf_512);
					if (res != board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
						break;
					}
					
					memcpy(&buf_512[((type*)buf_140)->quarter * debug_config::SD_PAGE_QUARTER_LENGTH], ((type*)buf_140)->data, debug_config::SD_PAGE_QUARTER_LENGTH);
					
					res = obc.SdPageWrite(static_cast<board::Sd>(sd_num), ((type*)buf_140)->addr, buf_512);
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
					debug.Read(buf_140, sizeof(type));
					sd_num = ((type*)buf_140)->sd_num;
					
					if ( sd_num  != static_cast<uint8_t>(board::Sd::kNum1) && sd_num  != static_cast<uint8_t>(board::Sd::kNum2) ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					res = obc.SdRangeErase(static_cast<board::Sd>(sd_num), ((type*)buf_140)->addr_start, ((type*)buf_140)->addr_end);
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
					debug.Read(buf_140, sizeof(type));
					sd_num = ((type*)buf_140)->sd_num;
					
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
					debug.Read(buf_140, sizeof(type));
					sd_num = ((type*)buf_140)->sd_num;
					
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
					debug.Read(buf_140, sizeof(type));
					mpu_num = ((type*)buf_140)->mpu_num;
					
					if ( mpu_num  != static_cast<uint8_t>(board::Magn::kNum1) && mpu_num  != static_cast<uint8_t>(board::Magn::kNum2) ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					res = obc.MagnRead(static_cast<board::Magn>(mpu_num), tme.sensors[mpu_num]);
					if (res == board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ACK));
						
						read_mpu_resp.A_X = tme.sensors[mpu_num].A_X;
						read_mpu_resp.A_Y = tme.sensors[mpu_num].A_Y;
						read_mpu_resp.A_Z = tme.sensors[mpu_num].A_Z;
						read_mpu_resp.B_X = tme.sensors[mpu_num].B_X;
						read_mpu_resp.B_Y = tme.sensors[mpu_num].B_Y;
						read_mpu_resp.B_Z = tme.sensors[mpu_num].B_Z;
						read_mpu_resp.G_X = tme.sensors[mpu_num].G_X;
						read_mpu_resp.G_Y = tme.sensors[mpu_num].G_Y;
						read_mpu_resp.G_Z = tme.sensors[mpu_num].G_Z;
						read_mpu_resp.T 	 = tme.sensors[mpu_num].T;
						
						debug.Write(&read_mpu_resp, sizeof(debug_config::READ_MPU_RESPONSE_t));
					} else {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
					}
					break;						
				}
				
				case debug_config::Commands::READ_TME: {
					using type = debug_config::READ_TME_REQUEST_t;
					debug.Read(buf_140, sizeof(type));
					sector_num = ((type*)buf_140)->sector_num;
										
					if ( sector_num  >= sectors_count ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					if ( obc.GetSdStatus(board::Sd::kNum1) == board::Status::kWorked ) {
						res = data_storage.template ReadTmeByTime<board::Sd::kNum1>(sector_num, ((type*)buf_140)->time, buf_512);
					}
					else if ( obc.GetSdStatus(board::Sd::kNum2) == board::Status::kWorked ) {
						res = data_storage.template ReadTmeByTime<board::Sd::kNum2>(sector_num, ((type*)buf_140)->time, buf_512);
					}
					else {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
					}
					
					if (res == board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ACK));
						sector_pars = app::DataStorageConfig::GetSectorPars(sector_num);
						memcpy(read_tme_resp.data, buf_512, sector_pars.tme_size);
						debug.Write(&read_tme_resp, sector_pars.tme_size);
					} else {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
					}
					break;
				}
				
				case debug_config::Commands::READ_TME_BUNCH: {
					using type = debug_config::READ_TME_BUNCH_REQUEST_t;
					debug.Read(buf_140, sizeof(type));
					sector_num = ((type*)buf_140)->sector_num;
										
					if ( sector_num  >= sectors_count ) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ERR));
						break;
					}
					
					if ( obc.GetSdStatus(board::Sd::kNum1) == board::Status::kWorked ) {
						res = data_storage.template ReadTmeBunch<board::Sd::kNum1>(sector_num, ((type*)buf_140)->time, ((type*)buf_140)->step, ((type*)buf_140)->qty, buf_512);
					}
					else if ( obc.GetSdStatus(board::Sd::kNum2) == board::Status::kWorked ) {
						res = data_storage.template ReadTmeBunch<board::Sd::kNum2>(sector_num, ((type*)buf_140)->time, ((type*)buf_140)->step, ((type*)buf_140)->qty, buf_512);
					}
					else {
						debug.WriteByte(static_cast<char>(debug_config::Response::NACK));
						debug.Write(&res, sizeof(res));
					}
					
					if (res == board::ERROR_CODE_OK) {
						debug.WriteByte(static_cast<char>(debug_config::Response::ACK));
						sector_pars = app::DataStorageConfig::GetSectorPars(sector_num);
						memcpy(read_tme_bunch_resp.data, buf_512, sector_pars.tme_size * ((type*)buf_140)->qty);
						debug.Write(&read_tme_bunch_resp, sector_pars.tme_size * ((type*)buf_140)->qty);
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
			allocator_.deallocate((void*)buf_140, BUF_140_SIZE);
			allocator_.deallocate((void*)buf_512, BUF_512_SIZE);
		}
		SleepUntil(25ms);
	}
}