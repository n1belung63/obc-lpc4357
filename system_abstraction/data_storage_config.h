#pragma once

#include <cstdint>

#include "../system_abstraction/board_api.h"

namespace app {

enum class Sector {
	ObcSensors = 0x00,
};

static constexpr uint32_t DATASTORAGE_SECTOR_COUNT = 1;

static constexpr uint32_t DATASTORAGE_SECTOR_OBC_SENSORS_RANGE_START = 0x00000000;
static constexpr uint32_t DATASTORAGE_SECTOR_OBC_SENSORS_RANGE_LENGTH = 0x00200000; // 1Gb
static constexpr uint32_t DATASTORAGE_SECTOR_OBC_SENSORS_SIZE = 40; //!!!!
static constexpr uint32_t DATASTORAGE_SECTOR_OBC_SENSORS_MAX_NUM = 512 / DATASTORAGE_SECTOR_OBC_SENSORS_SIZE;

typedef struct __attribute__ ((__packed__)) {
	uint32_t time;
	board::MagnData sensors[board::MAGN_COUNT];
} TObcMagnTme;

typedef struct __attribute__ ((__packed__)) {
	uint32_t tme_range_start;
	uint32_t tme_range_length;
	uint16_t tme_size;
	uint16_t tme_max_num_in_page;
	
	uint16_t tme_num_in_page;
	uint32_t page_to_write;
} TDataStorageSector;

}