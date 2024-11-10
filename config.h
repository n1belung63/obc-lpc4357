#pragma once

#include <cstdint>

namespace debug_config {

static constexpr uint16_t SD_PAGE_QUARTER_LENGTH = 128;
	
enum class Response {
	ACK = 0x06,
	NACK = 0x15,
	ERR = 0x45,
	WRONG = 0x57
};

enum class Commands {
	GET_SD_STATUS = 1,
	READ_SD = 2,
	WRITE_SD = 3,
	ERASE_SD = 4,
	BLOCK_SD = 5,
	UNBLOCK_SD = 6,
	READ_MPU = 7,
	READ_TME = 8,
	READ_TME_BUNCH = 9
};

typedef struct __attribute__ ((__packed__)) {
	uint8_t sd_num;
} GET_SD_STATUS_REQUEST_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t sd_num;
} GET_SD_STATUS_RESPONSE_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t sd_num;
	uint32_t addr;
	uint8_t quarter;
} READ_SD_REQUEST_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t data[128];
} READ_SD_RESPONSE_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t sd_num;
	uint32_t addr;
	uint8_t quarter;
	uint8_t data[128];
} WRITE_SD_REQUEST_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t sd_num;
	uint32_t addr_start;
	uint32_t addr_end;
} ERASE_SD_REQUEST_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t sd_num;
} BLOCK_SD_REQUEST_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t sd_num;
} UNBLOCK_SD_REQUEST_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t mpu_num;
} READ_MPU_REQUEST_t;

typedef struct __attribute__ ((__packed__)) {
	int16_t B_X;
	int16_t B_Y;
	int16_t B_Z;
	int16_t G_X;
	int16_t G_Y;
	int16_t G_Z;
	int16_t A_X;
	int16_t A_Y;
	int16_t A_Z;
	uint16_t T;
} READ_MPU_RESPONSE_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t sector_num;
	uint32_t time;
} READ_TME_REQUEST_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t data[128];
} READ_TME_RESPONSE_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t sector_num;
	uint32_t time;
	uint32_t step;
	uint32_t qty;
} READ_TME_BUNCH_REQUEST_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t data[512];
} READ_TME_BUNCH_RESPONSE_t;
	
}