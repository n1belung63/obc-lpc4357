#pragma once

#include "../board_settings.h"
#include "../system_abstraction/board_api.h"

#include "../singelton.h"

namespace board {

typedef struct StatusStruct {
	Status mpu[MAGN_COUNT];
	Status sd[SD_COUNT];
} TStatus;
	
struct MockBoard : public Singleton<MockBoard>, public IBoard {
	friend class Singleton<MockBoard>;
public:
	static constexpr int32_t ERROR_CODE_NOT_INITED = -40;

	virtual int32_t SdPageWrite(Sd num, uint32_t page_addr, uint8_t page[512]) override;
	virtual int32_t SdPageRead(Sd num, uint32_t page_addr, uint8_t page[512]) override;
	virtual int32_t SdRangeErase(Sd num, uint32_t start_addr, uint32_t end_addr) override;
	virtual int32_t MagnRead(Magn num, MagnData& data) override;
	virtual int32_t SdBlock(Sd num) override;
	virtual int32_t SdUnblock(Sd num) override;
	virtual Status GetMagnStatus(Magn id) override;
	virtual Status GetSdStatus(Sd id) override;

private:
	MockBoard();

	TStatus status_pool_;
};

}