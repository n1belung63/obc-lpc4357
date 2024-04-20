#pragma once

#include "../rtos_wrapper/rtos.h"
#include "../board.h"
#include "../application/data_storage_config.h"
#include "../application/data_storage.h"

using namespace std::chrono_literals;

template <typename TBoard>
class DataAcquisitionTask : public wrtos::Task<static_cast<std::size_t>(wrtos::StackDepth::minimal)> {
public:
	DataAcquisitionTask(app::DataStorage<TBoard>& data_storage);
  virtual void Execute() override;	
private:
	app::DataStorage<TBoard>& data_storage_;
};

template <typename TBoard>
DataAcquisitionTask<TBoard>::DataAcquisitionTask(app::DataStorage<TBoard>& data_storage)
	: data_storage_(data_storage)
	{ }

template <typename TBoard>
void DataAcquisitionTask<TBoard>::Execute() {
	TBoard& obc = TBoard::GetInstance();

	app::TObcMagnTme tme = {0};
	uint8_t num = 0;

	while(true) {
		for (num = 0; num < board::MAGN_COUNT; ++num) {
			if ( obc.GetMagnStatus(static_cast<board::Magn>(num)) == board::Status::kWorked ) {
				if ( obc.MagnRead(static_cast<board::Magn>(num), tme.sensors[num]) != board::ERROR_CODE_OK ) {
					memset(&tme.sensors[num], 0, sizeof(tme.sensors[num]));
				} else {
					if ( obc.GetSdStatus(board::Sd::kNum1) == board::Status::kWorked ) {
						//uint32_t magic = data_storage_.template DoMagic<board::Sd::kNum1>();
						data_storage_.template AddTmeToSd<board::Sd::kNum1>(app::Sector::ObcSensors, reinterpret_cast<uint8_t*>(&tme));
					}
					if ( obc.GetSdStatus(board::Sd::kNum2) == board::Status::kWorked ) {
						//uint32_t magic = data_storage_.template DoMagic<board::Sd::kNum2>();
						data_storage_.template AddTmeToSd<board::Sd::kNum2>(app::Sector::ObcSensors, reinterpret_cast<uint8_t*>(&tme));
					}
				}
			}
		}
		
		SleepUntil(250ms);
	}
}
