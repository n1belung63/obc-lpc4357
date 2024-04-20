#pragma once

#include <cstdint>

namespace board {
	
static constexpr int32_t ERROR_CODE_OK = 0;
	
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

static constexpr uint8_t SD_COUNT = 2;
static constexpr uint8_t MAGN_COUNT = 2;

enum class Sd { kNum1, kNum2 };
enum class Magn { kNum1, kNum2 };
enum class SdSector { kMagn };
	
class IBoard {
public:
	virtual int32_t SdPageWrite(Sd num, uint32_t page_addr, uint8_t page[512]) = 0;
  virtual int32_t SdPageRead(Sd num, uint32_t page_addr, uint8_t page[512]) = 0;
  virtual int32_t SdSectorErase(Sd num, SdSector sector_num) = 0;
  virtual int32_t MagnRead(Magn num, MagnData& data) = 0;
};
}