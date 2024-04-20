#pragma once

#include "LPC43xx.h"
#include "system_LPC43xx.h"

extern "C" {
#include "GPIO_LPC43xx.h"
#include "SCU_LPC43xx.h"
}

#include "board_settings.h"
#include "board_api.h"

#include "wrappers/uart_wrapper.h"
#include "wrappers/i2c_wrapper.h"
#include "wrappers/spi_wrapper.h"
#include "singelton.h"

namespace board {
enum class Status { kWorked, kFailed };

typedef struct StatusStruct {
	Status mpu[MAGN_COUNT];
	Status sd[SD_COUNT];
} TStatus;
	
struct Board : public Singleton<Board>, public IBoard {
	friend class Singleton<Board>;
public:
	static constexpr int32_t ERROR_CODE_NOT_INITED = -40;

	virtual int32_t SdPageWrite(Sd num, uint32_t page_addr, uint8_t page[512]) override;
  virtual int32_t SdPageRead(Sd num, uint32_t page_addr, uint8_t page[512]) override;
  virtual int32_t SdSectorErase(Sd num, SdSector sector_num) override;
  virtual int32_t MagnRead(Magn num, MagnData& data) override;

	inline __attribute__((always_inline)) Status GetMagnStatus(Magn id) {
		return status_pool_.mpu[static_cast<uint8_t>(id)];
	}
	inline __attribute__((always_inline)) Status GetSdStatus(Sd id) {
		return status_pool_.sd[static_cast<uint8_t>(id)];
	}

private:
	Board();
	void RiTimerConfig(uint32_t);

	TStatus status_pool_;
};

}