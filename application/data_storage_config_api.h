#pragma once

#include <cstdint>

namespace app {

typedef struct {
	uint32_t tme_range_start;
	uint32_t tme_range_length;
	uint16_t tme_size;
	uint16_t tme_max_num_in_page;
} TDataStorageSectorPars;

typedef struct {
	uint32_t page_to_write;
	uint16_t tme_num_in_page;
	uint16_t aligned_;
} TDataStorageSectorVars;

}