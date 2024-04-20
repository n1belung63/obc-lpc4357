#pragma once

#include <cstdint>

#include "../board_api.h"

namespace app {
	
enum class Sector {
	ObcSensors = 0x00,
};

typedef struct __attribute__ ((__packed__)) {
	board::MagnData sensors[board::MAGN_COUNT];
} TObcMagnTme;

typedef struct {
	uint32_t tme_range_start;
	uint32_t tme_range_length;
	uint16_t tme_size;
	uint16_t tme_max_num_in_page;
	
	uint16_t tme_num_in_page;
	uint32_t page_to_write;
} TDataStorageSector;

}