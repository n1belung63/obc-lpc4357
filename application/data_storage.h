#pragma once

#include "../rtos_wrapper/rtos.h"
#include "../board_api.h"
#include "../singelton.h"
#include "data_storage_config.h"

extern uint8_t buf_512[512];
extern volatile void _delay_ms(uint32_t delay);

namespace app {
	
static constexpr uint32_t DATASTORAGE_SECTOR_COUNT = 1;

static constexpr uint32_t DATASTORAGE_SECTOR_OBC_SENSORS_RANGE_START = 0x00000000;
static constexpr uint32_t DATASTORAGE_SECTOR_OBC_SENSORS_RANGE_LENGTH = 0x00200000; // 1Gb
static constexpr uint32_t DATASTORAGE_SECTOR_OBC_SENSORS_SIZE = 40; //!!!!
static constexpr uint32_t DATASTORAGE_SECTOR_OBC_SENSORS_MAX_NUM = 512 / DATASTORAGE_SECTOR_OBC_SENSORS_SIZE;

template <typename TBoard>
class DataStorage : public Singleton<DataStorage<TBoard>>  {
	friend class Singleton<DataStorage<TBoard>>;
public:
	static constexpr int32_t ERROR_CODE_OK = 0;
	static constexpr uint16_t ATTEMPTS_COUNT = 100;
	static constexpr uint8_t FREE_BYTE = 0xFF;
	
	template <board::Sd num>
	int32_t AddTmeToSd(app::Sector tme_num, uint8_t *tme);
	
private:
	DataStorage();

	TBoard& obc_;
	int32_t err_code_ = ERROR_CODE_OK;
	bool sd_writing_is_enabled_ = true;

	uint16_t attempts = 0;

	TDataStorageSector ds_arr_[DATASTORAGE_SECTOR_COUNT] = {
		{
			DATASTORAGE_SECTOR_OBC_SENSORS_RANGE_START,
			DATASTORAGE_SECTOR_OBC_SENSORS_RANGE_LENGTH,
			DATASTORAGE_SECTOR_OBC_SENSORS_SIZE,
			DATASTORAGE_SECTOR_OBC_SENSORS_MAX_NUM,
			0,
			0
		}
	};
	
	template <board::Sd num>
	inline __attribute__((always_inline)) int32_t ReadPageWithAddrToBuf(uint32_t addr, uint8_t* buf) {
		do {
			err_code_ = obc_.SdPageRead(num, addr, buf);
			_delay_ms(1);
		} while( err_code_ != ERROR_CODE_OK && ++attempts < ATTEMPTS_COUNT );
		
		if (err_code_ != ERROR_CODE_OK)
			return err_code_;
		
		return ERROR_CODE_OK;
	}

	template <board::Sd num>
	inline __attribute__((always_inline)) int32_t WriteBufToPageWithAddr(uint32_t addr, uint8_t* buf) {
		do {
			err_code_ = obc_.SdPageWrite(num, addr, buf);
			_delay_ms(1);
		} while( err_code_ != ERROR_CODE_OK && ++attempts < ATTEMPTS_COUNT );
		
		if (err_code_ != ERROR_CODE_OK)
			return err_code_;
		
		return ERROR_CODE_OK;
	}
	
	template <board::Sd num>
	inline __attribute__((always_inline)) void BlockSdUsing() {
		sd_writing_is_enabled_ = false;
	}
	
	template <board::Sd num>
	inline __attribute__((always_inline)) void UnblockSdUsing() {
		sd_writing_is_enabled_ = true;
	}
};

template <typename TBoard>
DataStorage<TBoard>::DataStorage() 
	: obc_(Singleton<TBoard>::GetInstance())
	{ }

template <typename TBoard>
template <board::Sd num>
int32_t DataStorage<TBoard>::AddTmeToSd(app::Sector tme_num, uint8_t *tme) {
	uint8_t tme_code = static_cast<uint8_t>(tme_num);
	
	if (ds_arr_[tme_code].tme_num_in_page == 0)
		memset((void*)buf_512, FREE_BYTE, sizeof(buf_512));
	
	if (ds_arr_[tme_code].tme_num_in_page > 0 && ds_arr_[tme_code].tme_num_in_page <= ds_arr_[tme_code].tme_max_num_in_page) {
		err_code_ = ReadPageWithAddrToBuf<num>(ds_arr_[tme_code].page_to_write, buf_512);
		if (err_code_ != ERROR_CODE_OK)
			return err_code_;
	}
	
	memcpy((void*)&buf_512[ds_arr_[tme_code].tme_num_in_page*ds_arr_[tme_code].tme_size], (void*)tme, ds_arr_[tme_code].tme_size);
	
	err_code_ = WriteBufToPageWithAddr<num>(ds_arr_[tme_code].page_to_write, buf_512);
	if (err_code_ != ERROR_CODE_OK)
		return err_code_;
			
	if (++ds_arr_[tme_code].tme_num_in_page == ds_arr_[tme_code].tme_max_num_in_page) {
		ds_arr_[tme_code].tme_num_in_page = 0;
		ds_arr_[tme_code].page_to_write++;
		
		if (ds_arr_[tme_code].page_to_write == ds_arr_[tme_code].tme_range_start + ds_arr_[tme_code].tme_range_length)
			ds_arr_[tme_code].page_to_write = ds_arr_[tme_code].tme_range_start;
	}
	
	return ERROR_CODE_OK;
}

}