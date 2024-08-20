#pragma once

#include "../system_abstraction/board_api.h"
#include "../system_abstraction/data_storage_config.h"

#include "../singelton.h"

#include <cstring>

extern uint8_t buf_512[512];
extern uint32_t buf_20[20];
extern volatile void _delay_ms(uint32_t delay);

namespace app {

template <typename TBoard>
class DataStorage : public Singleton<DataStorage<TBoard>>  {
	friend class Singleton<DataStorage<TBoard>>;
public:
	static constexpr int32_t ERROR_CODE_OK = 0;
	static constexpr uint16_t ATTEMPTS_COUNT = 100;
	static constexpr uint8_t FREE_BYTE = 0xFF;
	
	template <board::Sd num>
	int32_t AddTmeToSd(app::Sector tme_num, uint8_t *tme_data);

	template <board::Sd num>
	int32_t ReadTmeByTime(app::Sector tme_num, uint32_t tme_time, uint8_t *tme_data);

private:
	DataStorage();

	template <board::Sd num>
	int32_t FindPageToRead(app::Sector tme_num, uint32_t tme_time, uint32_t *page_to_read);

	TBoard& obc_;
	int32_t err_code_ = ERROR_CODE_OK;

	uint16_t tme_num_in_page_ = 0;
	uint32_t page_to_read_ = 0;
	uint16_t attempts_ = 0;

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
		} while( err_code_ != ERROR_CODE_OK && ++attempts_ < ATTEMPTS_COUNT );
		
		if (err_code_ != ERROR_CODE_OK)
			return err_code_;
		
		return ERROR_CODE_OK;
	}

	template <board::Sd num>
	inline __attribute__((always_inline)) int32_t WriteBufToPageWithAddr(uint32_t addr, uint8_t* buf) {
		do {
			err_code_ = obc_.SdPageWrite(num, addr, buf);
			_delay_ms(1);
		} while( err_code_ != ERROR_CODE_OK && ++attempts_ < ATTEMPTS_COUNT );
		
		if (err_code_ != ERROR_CODE_OK)
			return err_code_;
		
		return ERROR_CODE_OK;
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
	
	memcpy((void*)&buf_512[ds_arr_[tme_code].tme_num_in_page * ds_arr_[tme_code].tme_size], (void*)tme, ds_arr_[tme_code].tme_size);
	
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

template <typename TBoard>
template <board::Sd num>
int32_t DataStorage<TBoard>::ReadTmeByTime(app::Sector tme_num, uint32_t tme_time, uint8_t *tme_data) {
	uint8_t tme_code = static_cast<uint8_t>(tme_num);

	uint32_t page_to_read;
	uint8_t i;
	
	err_code_ = FindPageToRead(tme_num, tme_time, (uint32_t*)&page_to_read_);
	if (err_code_ != ERROR_CODE_OK)
		return err_code_;
	
	for(i = 0; i < ds_arr_[tme_code].tme_max_num_in_page; i++)
		memcpy((void*)&buf_20[i], (void*)&buf_512[i * ds_arr_[tme_code].tme_size], sizeof(uint32_t));

	for(i = 0; i < ds_arr_[tme_code].tme_max_num_in_page; i++)
		if (buf_20[i] == tme_time)
			page_to_read_ = i;
		
	memcpy((void*)tme_data, (void*)&buf_512[tme_num_in_page_ * ds_arr_[tme_code].tme_size], ds_arr_[tme_code].tme_size);
			
	return ERROR_CODE_OK;
}

template <typename TBoard>
template <board::Sd num>
int32_t DataStorage<TBoard>::FindPageToRead(app::Sector tme_num, uint32_t tme_time, uint32_t *page_to_read) {
	uint8_t tme_code = static_cast<uint8_t>(tme_num);
	
	// todo: Add magic
	
	return ERROR_CODE_OK;
}

}