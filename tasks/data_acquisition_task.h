#pragma once

#include "../rtos_wrapper/rtos.h"
#include "../board_api.h"
#include "../application/data_storage_config.h"
#include "../application/data_storage.h"

using namespace std::chrono_literals;

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
	TBoard& obc = TBoard::GetInstance();
	app::DataStorage<TBoard>& data_storage = app::DataStorage<TBoard>::GetInstance();
	
	app::TObcMagnTme tme = {0};

	while(true) {
		for ( uint8_t num = 0; num < board::MAGN_COUNT; ++num ) {
			if ( obc.GetMagnStatus(static_cast<board::Magn>(num)) == board::Status::kWorked ) {
				if ( obc.MagnRead(static_cast<board::Magn>(num), tme.sensors[num]) == board::ERROR_CODE_OK ) {
					if ( obc.GetSdStatus(board::Sd::kNum1) == board::Status::kWorked ) {
						data_storage.template AddTmeToSd<board::Sd::kNum1>(app::Sector::ObcSensors, reinterpret_cast<uint8_t*>(&tme));
					}
					if ( obc.GetSdStatus(board::Sd::kNum2) == board::Status::kWorked ) {
						data_storage.template AddTmeToSd<board::Sd::kNum2>(app::Sector::ObcSensors, reinterpret_cast<uint8_t*>(&tme));
					}
				} else {
					memset(&tme.sensors[num], 0, sizeof(tme.sensors[num]));
				}
			}
		}
		SleepUntil(250ms);
	}
}
