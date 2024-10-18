#pragma once

#include <cstdint>

namespace board {
	
static constexpr int32_t ERROR_CODE_OK = 0;
static constexpr uint16_t SD_PAGE_SIZE = 512;

static constexpr uint8_t SD_COUNT = 2;
static constexpr uint8_t MAGN_COUNT = 2;

typedef struct {
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
} MagnData;

enum class Sd { kNum1, kNum2 };
enum class Magn { kNum1, kNum2 };
enum class Status { kWorked, kFailed, kBlocked };
	
class IBoard {
public:
	virtual int32_t SdPageWrite(Sd num, uint32_t page_addr, uint8_t page[SD_PAGE_SIZE]) = 0;
	virtual int32_t SdPageRead(Sd num, uint32_t page_addr, uint8_t page[SD_PAGE_SIZE]) = 0;
	virtual int32_t SdRangeErase(Sd num, uint32_t start_addr, uint32_t end_addr) = 0;
	virtual int32_t SdBlock(Sd num) = 0;
	virtual int32_t SdUnblock(Sd num) = 0;
	virtual int32_t MagnRead(Magn num, MagnData& data) = 0;
	virtual Status GetMagnStatus(Magn id) = 0;
	virtual Status GetSdStatus(Sd id) = 0;
};
}