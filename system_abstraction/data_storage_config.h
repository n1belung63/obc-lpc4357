#pragma once

#include "application/data_storage_config_api.h"
#include "system_abstraction/board_api.h"
#include "board_settings.h"

#include <cstdint>
#include <cassert>
#include <array>

namespace app {

typedef struct __attribute__ ((__packed__)) {
	uint32_t time;
	board::MagnData sensors[board::MAGN_COUNT];
} TObcMagnTme;

struct DataStorageConfig {
private:
	static constexpr uint32_t SECTORS_COUNT_ = 1;

	static constexpr uint32_t SECTOR_OBC_SENSORS_RANGE_START_ = 0x00000000;
	static constexpr uint32_t SECTOR_OBC_SENSORS_RANGE_LENGTH_ = 0x00200000; // 1Gb
	static constexpr uint32_t SECTOR_OBC_SENSORS_SIZE_ = sizeof(TObcMagnTme);
	static constexpr uint32_t SECTOR_OBC_SENSORS_MAX_NUM_ = board::SD_PAGE_SIZE / SECTOR_OBC_SENSORS_SIZE_;

	static constexpr std::array<TDataStorageSectorPars, SECTORS_COUNT_> SECTORS_PARS_ = {
		{
			SECTOR_OBC_SENSORS_RANGE_START_,
			SECTOR_OBC_SENSORS_RANGE_LENGTH_,
			SECTOR_OBC_SENSORS_SIZE_,
			SECTOR_OBC_SENSORS_MAX_NUM_,
		}
	};

public:
	static constexpr  TDataStorageSectorPars GetSectorPars(uint8_t sector_num) {
		assert(sector_num < SECTORS_COUNT_);
		return SECTORS_PARS_.at(sector_num);
	}

	static constexpr uint32_t GetSectorsCount() {
		return SECTORS_COUNT_;
	}

	enum class Sector {
		ObcSensors = 0x00,
	};
};

}
