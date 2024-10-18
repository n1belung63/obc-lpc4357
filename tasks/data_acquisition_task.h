#pragma once

#include "rtos_wrapper/rtos.h"

#include "system_abstraction/board_api.h"

#include "board_settings.h"
#include "system_abstraction/data_storage_config.h"
#include "application/data_storage.h"

#include "pool_allocator_port.h"

using namespace std::chrono_literals;
using namespace wrtos;

template <typename TBoard>
class DataAcquisitionTask : public wrtos::Task<static_cast<std::size_t>(wrtos::StackDepth::minimal)> {
public:
	DataAcquisitionTask();
	virtual void Execute() override;
};

template <typename TBoard>
DataAcquisitionTask<TBoard>::DataAcquisitionTask() { }

template <typename TBoard>
void DataAcquisitionTask<TBoard>::Execute() {
	using namespace allocator;
	using Sector = app::DataStorageConfig::Sector;
	TBoard& obc = TBoard::GetInstance();
	app::DataStorage<TBoard,PoolAllocatorPort,app::DataStorageConfig>& data_storage = app::DataStorage<TBoard,PoolAllocatorPort,app::DataStorageConfig>::GetInstance();
		
	app::TObcMagnTme tme = {0};
	// app::TObcMagnTme* tme = static_cast<app::TObcMagnTme*>(allocator_.allocate(sizeof(app::TObcMagnTme)));

	while(true) {
		for ( uint8_t num = 0; num < board::MAGN_COUNT; ++num ) {
			if ( obc.GetMagnStatus(static_cast<board::Magn>(num)) == board::Status::kWorked ) {
				if ( obc.MagnRead(static_cast<board::Magn>(num), tme.sensors[num]) != board::ERROR_CODE_OK ) {
					memset(&tme.sensors[num], 0, sizeof(tme.sensors[num]));
				}
			}
		}
		tme.time = Rtos::GetSchedulerTime();
		if ( obc.GetSdStatus(board::Sd::kNum1) == board::Status::kWorked ) {
			data_storage.template AddTmeToSd<board::Sd::kNum1,Sector::ObcSensors>(reinterpret_cast<uint8_t*>(&tme));
		}
		if ( obc.GetSdStatus(board::Sd::kNum2) == board::Status::kWorked ) {
			data_storage.template AddTmeToSd<board::Sd::kNum2,Sector::ObcSensors>(reinterpret_cast<uint8_t*>(&tme));
		}
		SleepUntil(250ms);
	}
}
