#pragma once

#include "singelton/singelton.h"
#include "pool-allocator/pool_allocator.h"

#include "system_abstraction/board_api.h"
#include "application/data_storage_config_api.h"
#include "pool-allocator/pool_allocator_port_api.h"

#include <cstring>

extern volatile void _delay_ms(uint32_t delay);

template<typename TPoolAllocatorPort>
using DefaultAllocator = allocator::PoolAllocator<allocator::DefaultBlockSize,allocator::DefaultBlockCount, TPoolAllocatorPort>;

namespace app {

template <typename T, typename TPoolAllocatorPort>
class RAIIAllocatedBuffer {
public:
	RAIIAllocatedBuffer(uint32_t size) 
	: allocator_(Singleton<DefaultAllocator<TPoolAllocatorPort>>::GetInstance()),
	size_(size)
	{
		data = static_cast<T*>(allocator_.allocate(size_));
	}
	~RAIIAllocatedBuffer() {
		allocator_.deallocate((void*)data, size_);
	}
	T* data;
private:
	DefaultAllocator<TPoolAllocatorPort>& allocator_;
	uint32_t size_;
};
	
template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
class DataStorage : public Singleton<DataStorage<TBoard, TPoolAllocatorPort, TConfig>>  {
	friend class Singleton<DataStorage<TBoard, TPoolAllocatorPort, TConfig>>;
public:
	static constexpr int32_t ERROR_CODE_OK = 0;
	static constexpr int32_t ERROR_CODE_NO_DATA = -1;
	static constexpr int32_t ERROR_CODE_GREATER_THEN_NEWEST_TIME = -2;
	static constexpr int32_t ERROR_CODE_LESS_THEN_OLDEST_TIME = -3;
	static constexpr int32_t ERROR_CODE_TME_NOT_FOUND = -4;

	static constexpr uint16_t ATTEMPTS_COUNT = 100;
	static constexpr uint8_t FREE_BYTE = 0xFF;
	static constexpr uint16_t BUFFER_SIZE = 512;

	static constexpr uint32_t NULL_TIME = UINT32_MAX;
	static constexpr uint32_t ZERO_TIME = 0;
	
	template <board::Sd num, auto sector_num>
	int32_t AddTmeToSd(uint8_t *tme_data);

	template <board::Sd num, auto sector_num>
	int32_t ReadTmeByTime(uint32_t tme_time, uint8_t *tme_data);

	template <board::Sd num, auto sector_num>
	int32_t InitSector();

	template <board::Sd num, auto sector_num>
	int32_t FindPageToRead(uint32_t tme_time, uint32_t *page_to_read);

	template <board::Sd num, auto sector_num>
	int32_t FindPageToWrite(uint32_t *page_to_write);

	template <board::Sd num, auto sector_num>
	int32_t ReadTmeBunch(uint32_t tme_start_time, uint32_t tme_step, uint32_t tme_qty, uint8_t *tmes_data);

	template <board::Sd num, auto sector_num>
	inline __attribute__((always_inline)) TDataStorageSectorVars GetSectorVars() {
		assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());
		assert(static_cast<uint8_t>(num) < board::SD_COUNT);
		return ds_vars_arr_[static_cast<uint8_t>(num)][static_cast<uint8_t>(sector_num)];
	}
	
	template <board::Sd num>
	inline __attribute__((always_inline)) int32_t ReadTmeByTime(uint8_t sector_num_, uint32_t tme_time, uint8_t *tme_data) {
		constexpr auto last_sector_num = static_cast<typename TConfig::Sector>(TConfig::GetSectorsCount() - 1);
		return ReadTmeByTimeSupport<num, last_sector_num>(sector_num_, tme_time, tme_data);
	}
	
	template <board::Sd num>
	inline __attribute__((always_inline)) int32_t ReadTmeBunch(uint8_t sector_num_, uint32_t tme_start_time, uint32_t tme_step, uint32_t tme_qty, uint8_t *tmes_data) {
		constexpr auto last_sector_num = static_cast<typename TConfig::Sector>(TConfig::GetSectorsCount() - 1);
		return ReadTmeBunchSupport<num, last_sector_num>(sector_num_, tme_start_time, tme_step, tme_qty, tmes_data);
	}

private:
	DataStorage();
	
	template <board::Sd num, auto sector_num>
	int32_t GetAllTimesInPage(uint32_t addr, uint32_t *times);

	template <board::Sd num, auto sector_num>
	uint32_t FindNumInBufByTime(uint8_t* buf, uint32_t tme_time);

	template <board::Sd num, auto sector_num>
	int32_t InitSectorSupport();
	
	template <board::Sd num, auto sector_num>
	int32_t ReadTmeByTimeSupport(uint8_t sector_num_, uint32_t tme_time, uint8_t *tme_data);
	
	template <board::Sd num, auto sector_num>
	int32_t ReadTmeBunchSupport(uint8_t sector_num_, uint32_t tme_start_time, uint32_t tme_step, uint32_t tme_qty, uint8_t *tmes_data);

	TBoard& obc_;
	DefaultAllocator<TPoolAllocatorPort>& allocator_;
	
	int32_t err_code_ = ERROR_CODE_OK;

	uint16_t tme_num_in_page_ = 0;
	uint32_t page_to_read_ = 0;
	uint8_t tme_code_ = 0;
	uint8_t sd_num_ = 0;
	TDataStorageSectorPars sector_pars_;

	TDataStorageSectorVars ds_vars_arr_[board::SD_COUNT][TConfig::GetSectorsCount()] = {0};
	
	template <board::Sd num>
	inline __attribute__((always_inline)) int32_t ReadPageWithAddrToBuf(uint32_t addr, uint8_t* buf) {
		uint16_t attempts = 0;
		do {
			err_code_ = obc_.SdPageRead(num, addr, buf);
			_delay_ms(1);
		} while( err_code_ != ERROR_CODE_OK && ++attempts < ATTEMPTS_COUNT );
		
		return err_code_;
	}

	template <board::Sd num>
	inline __attribute__((always_inline)) int32_t WriteBufToPageWithAddr(uint32_t addr, uint8_t* buf) {
		uint16_t attempts = 0;
		do {
			err_code_ = obc_.SdPageWrite(num, addr, buf);
			_delay_ms(1);
		} while( err_code_ != ERROR_CODE_OK && ++attempts < ATTEMPTS_COUNT );
	
		return err_code_;
	}

	template <board::Sd num, auto sector_num>
	inline __attribute__((always_inline)) bool IsThisTimeInArray(uint32_t* time_buf, uint32_t tme_time) {
		for (uint8_t i = 0; i < TConfig::GetSectorPars(static_cast<uint8_t>(sector_num)).tme_max_num_in_page; i++)
			if (tme_time == time_buf[i])
				return true;
		return false;
	}

	template <board::Sd num, auto sector_num>
	inline __attribute__((always_inline)) uint32_t FindMaxTimeInTimes(uint32_t* time_buf, uint32_t time0) {
		uint32_t max = time0;	
		for (uint8_t i = 0; i < TConfig::GetSectorPars(static_cast<uint8_t>(sector_num)).tme_max_num_in_page; i++)
			if ( time_buf[i] > max && time_buf[i] != ZERO_TIME && time_buf[i] != NULL_TIME )
				max = time_buf[i];
		return max;
	}

	template <board::Sd num, auto sector_num>
	inline __attribute__((always_inline)) uint32_t FindMinTimeInTimes(uint32_t* time_buf, uint32_t time0) {
		uint32_t min = time0;
		for (uint8_t i = 0; i < TConfig::GetSectorPars(static_cast<uint8_t>(sector_num)).tme_max_num_in_page; i++)
			if ( time_buf[i] < min && time_buf[i] != ZERO_TIME && time_buf[i] != NULL_TIME )
				min = time_buf[i];
		return min;
	}

	template <board::Sd num, auto sector_num>
	inline __attribute__((always_inline)) int32_t FindNewestTime(uint32_t* newest_time) {
		assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());

		tme_code_ = static_cast<uint8_t>(sector_num);
		sd_num_ = static_cast<uint8_t>(num);
		sector_pars_ = TConfig::GetSectorPars(tme_code_);
		
		uint32_t page_to_write_left;
		
		if (ds_vars_arr_[sd_num_][tme_code_].page_to_write == sector_pars_.tme_range_start) {
			page_to_write_left = sector_pars_.tme_range_start + sector_pars_.tme_range_length - 1;
		} else {
			page_to_write_left = ds_vars_arr_[sd_num_][tme_code_].page_to_write - 1;
		}
		
		RAIIAllocatedBuffer<uint32_t,TPoolAllocatorPort> time_buf(sector_pars_.tme_max_num_in_page * sizeof(uint32_t));
		
		err_code_ = GetAllTimesInPage<num,sector_num>(page_to_write_left, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}
		*newest_time = FindMaxTimeInTimes<num,sector_num>(time_buf.data, ZERO_TIME);
		
		err_code_ = GetAllTimesInPage<num,sector_num>(ds_vars_arr_[sd_num_][tme_code_].page_to_write, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}
		*newest_time = FindMaxTimeInTimes<num,sector_num>(time_buf.data, *newest_time);
		
		return ERROR_CODE_OK;
	}

	template <board::Sd num, auto sector_num>
	inline __attribute__((always_inline)) int32_t FindOldestTime(uint32_t* oldest_time) {
		assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());

		tme_code_ = static_cast<uint8_t>(sector_num);
		sd_num_ = static_cast<uint8_t>(num);
		sector_pars_ = TConfig::GetSectorPars(tme_code_);

		uint32_t page_to_write_right;
		if (ds_vars_arr_[sd_num_][tme_code_].page_to_write == sector_pars_.tme_range_start + sector_pars_.tme_range_length - 1) {
			page_to_write_right = sector_pars_.tme_range_start;
		} else {
			page_to_write_right = ds_vars_arr_[sd_num_][tme_code_].page_to_write + 1;
		}

		RAIIAllocatedBuffer<uint32_t,TPoolAllocatorPort> time_buf(sector_pars_.tme_max_num_in_page * sizeof(uint32_t));
				
		err_code_ = GetAllTimesInPage<num,sector_num>(sector_pars_.tme_range_start, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}		
		*oldest_time = FindMinTimeInTimes<num,sector_num>(time_buf.data, NULL_TIME);
			
		err_code_ = GetAllTimesInPage<num,sector_num>(page_to_write_right, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}
		*oldest_time = FindMinTimeInTimes<num,sector_num>(time_buf.data, *oldest_time);

		return ERROR_CODE_OK;
	}
};

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
DataStorage<TBoard, TPoolAllocatorPort, TConfig>::DataStorage() 
	: obc_(Singleton<TBoard>::GetInstance()),
	allocator_(Singleton<DefaultAllocator<TPoolAllocatorPort>>::GetInstance())
	{
		constexpr auto last_sector_num = static_cast<typename TConfig::Sector>(TConfig::GetSectorsCount() - 1);
		InitSectorSupport<board::Sd::kNum1, last_sector_num>();
		InitSectorSupport<board::Sd::kNum2, last_sector_num>();
	}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
int32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::InitSectorSupport() {
	InitSector<num,sector_num>();
	if constexpr(static_cast<uint8_t>(sector_num) > 0) {
		constexpr auto prev_sector_num = static_cast<typename TConfig::Sector>(static_cast<uint8_t>(sector_num) - 1);
		InitSectorSupport<num,prev_sector_num>();
	}

	return ERROR_CODE_OK;
}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
int32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::ReadTmeByTimeSupport(uint8_t sector_num_, uint32_t tme_time, uint8_t *tme_data) {
	if (sector_num_ == static_cast<uint8_t>(sector_num)) {
		return ReadTmeByTime<num,sector_num>(tme_time, tme_data);
	}
	
	if constexpr(static_cast<uint8_t>(sector_num) > 0) {
		constexpr auto prev_sector_num = static_cast<typename TConfig::Sector>(static_cast<uint8_t>(sector_num) - 1);
		ReadTmeByTimeSupport<num,prev_sector_num>(sector_num_, tme_time, tme_data);
	}
	
	return ERROR_CODE_NO_DATA;
}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
int32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::ReadTmeBunchSupport(uint8_t sector_num_, uint32_t tme_start_time, uint32_t tme_step, uint32_t tme_qty, uint8_t *tmes_data) {
	if (sector_num_ == static_cast<uint8_t>(sector_num)) {
		return ReadTmeBunch<num,sector_num>(tme_start_time, tme_step, tme_qty, tmes_data);
	}
	
	if constexpr(static_cast<uint8_t>(sector_num) > 0) {
		constexpr auto prev_sector_num = static_cast<typename TConfig::Sector>(static_cast<uint8_t>(sector_num) - 1);
		ReadTmeBunchSupport<num,prev_sector_num>(sector_num_, tme_start_time, tme_step, tme_qty, tmes_data);
	}
	
	return ERROR_CODE_NO_DATA;
}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
int32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::InitSector() {
	assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());

	tme_code_ = static_cast<uint8_t>(sector_num);
	sd_num_ = static_cast<uint8_t>(num);
	sector_pars_ = TConfig::GetSectorPars(tme_code_);
	
	err_code_ = FindPageToWrite<num,sector_num>(&ds_vars_arr_[sd_num_][tme_code_].page_to_write);
	if (err_code_ != ERROR_CODE_OK) {
		return err_code_;
	}

	RAIIAllocatedBuffer<uint32_t,TPoolAllocatorPort> time_buf(sector_pars_.tme_max_num_in_page * sizeof(uint32_t));

	err_code_ = GetAllTimesInPage<num,sector_num>(ds_vars_arr_[sd_num_][tme_code_].page_to_write, time_buf.data);
	if (err_code_ != ERROR_CODE_OK) {
		return err_code_;
	}
	ds_vars_arr_[sd_num_][tme_code_].tme_num_in_page = 0;
	if (IsThisTimeInArray<num,sector_num>(time_buf.data, NULL_TIME)) {
		for(uint8_t i = 0; i < sector_pars_.tme_max_num_in_page; i++) {
			if (time_buf.data[i] == NULL_TIME) {
				ds_vars_arr_[sd_num_][tme_code_].tme_num_in_page = i;
				break;
			}
		}
	}

	return ERROR_CODE_OK;
}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
int32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::AddTmeToSd(uint8_t *tme) {
	assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());

	RAIIAllocatedBuffer<uint8_t,TPoolAllocatorPort> buf(BUFFER_SIZE);

	tme_code_ = static_cast<uint8_t>(sector_num);
	sd_num_ = static_cast<uint8_t>(num);
	sector_pars_ = TConfig::GetSectorPars(tme_code_);
	
	if (ds_vars_arr_[sd_num_][tme_code_].tme_num_in_page == 0) {
		memset((void*)buf.data, FREE_BYTE, BUFFER_SIZE);
	}
	
	if (ds_vars_arr_[sd_num_][tme_code_].tme_num_in_page > 0 && ds_vars_arr_[sd_num_][tme_code_].tme_num_in_page <= sector_pars_.tme_max_num_in_page) {
		err_code_ = ReadPageWithAddrToBuf<num>(ds_vars_arr_[sd_num_][tme_code_].page_to_write, buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}
	}
	
	memcpy((void*)&buf.data[ds_vars_arr_[sd_num_][tme_code_].tme_num_in_page * sector_pars_.tme_size], (void*)tme, sector_pars_.tme_size);
	
	err_code_ = WriteBufToPageWithAddr<num>(ds_vars_arr_[sd_num_][tme_code_].page_to_write, buf.data);
	if (err_code_ != ERROR_CODE_OK) {
		return err_code_;
	}
			
	if (++ds_vars_arr_[sd_num_][tme_code_].tme_num_in_page == sector_pars_.tme_max_num_in_page) {
		ds_vars_arr_[sd_num_][tme_code_].tme_num_in_page = 0;
		ds_vars_arr_[sd_num_][tme_code_].page_to_write++;
		
		if (ds_vars_arr_[sd_num_][tme_code_].page_to_write == sector_pars_.tme_range_start + sector_pars_.tme_range_length) {
			ds_vars_arr_[sd_num_][tme_code_].page_to_write = sector_pars_.tme_range_start;
		}
	}
	
	return ERROR_CODE_OK;
}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
int32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::ReadTmeByTime(uint32_t tme_time, uint8_t *tme_data) {
	assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());

	tme_code_ = static_cast<uint8_t>(sector_num);
	sd_num_ = static_cast<uint8_t>(num);

	err_code_ = FindPageToRead<num,sector_num>(tme_time, &page_to_read_); // todo: add tme_num_in_page_ and buf.data to argument???
	if (err_code_ != ERROR_CODE_OK) {
		return err_code_;
	}
	RAIIAllocatedBuffer<uint8_t,TPoolAllocatorPort> buf(BUFFER_SIZE);
	err_code_ = ReadPageWithAddrToBuf<num>(page_to_read_, buf.data);
	if (err_code_ != ERROR_CODE_OK) {
		return err_code_;
	}
	tme_num_in_page_ = FindNumInBufByTime<num,sector_num>(buf.data, tme_time);
	sector_pars_ = TConfig::GetSectorPars(tme_code_);
	memcpy((void*)tme_data, (void*)&buf.data[tme_num_in_page_ * sector_pars_.tme_size], sector_pars_.tme_size);
					
	return ERROR_CODE_OK;
}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
int32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::FindPageToRead(uint32_t tme_time, uint32_t *page_to_read) {
	assert(tme_time != ZERO_TIME && tme_time != NULL_TIME);
	assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());

	tme_code_ = static_cast<uint8_t>(sector_num);
	sd_num_ = static_cast<uint8_t>(num);
	sector_pars_ = TConfig::GetSectorPars(tme_code_);
	
	RAIIAllocatedBuffer<uint32_t,TPoolAllocatorPort> time_buf(sector_pars_.tme_max_num_in_page * sizeof(uint32_t));

	uint32_t tme_range_start = sector_pars_.tme_range_start;
	uint32_t tme_range_end = tme_range_start + sector_pars_.tme_range_length - 1;
	
	uint32_t page_to_write = ds_vars_arr_[sd_num_][tme_code_].page_to_write;
	uint32_t page_to_write_left = (page_to_write == tme_range_start) ? tme_range_end : page_to_write - 1;
	uint32_t page_to_write_right = (page_to_write == tme_range_end) ? tme_range_start : page_to_write + 1;

	uint32_t time_start, time_end, time_oldest, time_newest;

	//  Find oldest tme time
	{
		err_code_ = GetAllTimesInPage<num,sector_num>(tme_range_start, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}		
		if (IsThisTimeInArray<num,sector_num>(time_buf.data, tme_time)) {
			*page_to_read = tme_range_start;
			return ERROR_CODE_OK;
		}	
		time_start = FindMinTimeInTimes<num,sector_num>(time_buf.data, NULL_TIME);

		if (time_start == NULL_TIME) {		
			return ERROR_CODE_NO_DATA;
		}

		err_code_ = GetAllTimesInPage<num,sector_num>(page_to_write_right, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}
		if (IsThisTimeInArray<num,sector_num>(time_buf.data, tme_time)) {
			*page_to_read = page_to_write_right;
			return ERROR_CODE_OK;
		}
		time_oldest = FindMinTimeInTimes<num,sector_num>(time_buf.data, time_start);
	
		if (tme_time < time_oldest) {		
			return ERROR_CODE_LESS_THEN_OLDEST_TIME;
		}
	}

	enum Form { form_1, form_2 };
	Form form = (time_oldest == time_start) ? form_1 : form_2;

	//  Find newest tme time
	{
		err_code_ = GetAllTimesInPage<num,sector_num>(page_to_write_left, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}	
		if (IsThisTimeInArray<num,sector_num>(time_buf.data, tme_time)) {
			*page_to_read = page_to_write_left;
			return ERROR_CODE_OK;
		}
		time_newest = FindMaxTimeInTimes<num,sector_num>(time_buf.data, ZERO_TIME);
	
		err_code_ = GetAllTimesInPage<num,sector_num>(page_to_write, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}
		if (IsThisTimeInArray<num,sector_num>(time_buf.data, tme_time)) {
			*page_to_read = page_to_write;
			return ERROR_CODE_OK;
		}
		time_newest = FindMaxTimeInTimes<num,sector_num>(time_buf.data, time_newest);
	
		if (time_newest == ZERO_TIME) {	
			return ERROR_CODE_NO_DATA;
		}	
		if (tme_time > time_newest) {
			return ERROR_CODE_GREATER_THEN_NEWEST_TIME;
		}
	}

	//  Check the last page
	if (form == form_2) {
		err_code_ = GetAllTimesInPage<num,sector_num>(tme_range_end, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}
		if (IsThisTimeInArray<num,sector_num>(time_buf.data, tme_time)) {
			*page_to_read = tme_range_end;
			return ERROR_CODE_OK;
		}
		time_end = time_buf.data[ sector_pars_.tme_max_num_in_page - 1 ];
	}

	//  Choose the borders
	uint32_t L, R, m;
	if (tme_time > time_start && tme_time < time_newest) {
		L = tme_range_start;
		R = page_to_write_left;
	}
	if (form == form_2) {
		if (tme_time < time_start && tme_time > time_oldest && tme_time < time_end) {
			L = page_to_write_right;
			R = tme_range_end;
		}
	}
	
	//  Binary search
	while (L <= R) {
		m = (L + R) / 2;
		
		err_code_ = GetAllTimesInPage<num,sector_num>(m, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}
		
		if (IsThisTimeInArray<num,sector_num>(time_buf.data, tme_time)) {
			*page_to_read = m;
			return ERROR_CODE_OK;
		}

		if (time_buf.data[0] < tme_time) {
			L = m + 1;
		} else {
			R = m - 1;
		}
	}

	return ERROR_CODE_TME_NOT_FOUND;
}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
int32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::FindPageToWrite(uint32_t *page_to_write) {
	assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());

	tme_code_ = static_cast<uint8_t>(sector_num);
	sd_num_ = static_cast<uint8_t>(num);
	sector_pars_ = TConfig::GetSectorPars(tme_code_);
	
	uint32_t L = sector_pars_.tme_range_start;
	uint32_t R = sector_pars_.tme_range_start + sector_pars_.tme_range_length - 1;
	uint32_t m, m0 = 0, time_m;

	RAIIAllocatedBuffer<uint32_t,TPoolAllocatorPort> time_buf(sector_pars_.tme_max_num_in_page * sizeof(uint32_t));
	
	err_code_ = GetAllTimesInPage<num,sector_num>(R, time_buf.data);
	if (err_code_ != ERROR_CODE_OK) {
		return err_code_;
	}
	uint32_t time_R = FindMinTimeInTimes<num,sector_num>(time_buf.data, time_buf.data[0]);
	
	while (L < R) {
		m = (L + R) / 2;
		
		err_code_ = GetAllTimesInPage<num,sector_num>(m, time_buf.data);
		if (err_code_ != ERROR_CODE_OK) {
			return err_code_;
		}
		time_m = FindMinTimeInTimes<num,sector_num>(time_buf.data, time_buf.data[0]);

		if (m == m0) {
			if (IsThisTimeInArray<num,sector_num>(time_buf.data, NULL_TIME)) {
				*page_to_write = m;
				return ERROR_CODE_OK;
			}
			break;
		}	
		m0 = m;

		if (time_m > time_R) {
			L = m;
		} else	{
			R = m;
		}
	}
	
	*page_to_write = R;
	return ERROR_CODE_OK;
}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
int32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::GetAllTimesInPage(uint32_t addr, uint32_t *times) {	
	assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());

	tme_code_ = static_cast<uint8_t>(sector_num);
	sd_num_ = static_cast<uint8_t>(num);
	sector_pars_ = TConfig::GetSectorPars(tme_code_);
	
	assert(sector_pars_.tme_range_start <= addr && 
		addr <= sector_pars_.tme_range_start + sector_pars_.tme_range_length - 1);

	RAIIAllocatedBuffer<uint8_t,TPoolAllocatorPort> buf(BUFFER_SIZE);
	err_code_ = ReadPageWithAddrToBuf<num>(addr, buf.data);
	if (err_code_ != ERROR_CODE_OK) {
		return err_code_;
	}
	for(uint16_t i = 0; i < sector_pars_.tme_max_num_in_page; ++i) {
		memcpy((void*)&times[i], (void*)&buf.data[i * sector_pars_.tme_size], sizeof(uint32_t));
	}
		
	return ERROR_CODE_OK;
}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
uint32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::FindNumInBufByTime(uint8_t* buf, uint32_t tme_time) {
	assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());

	tme_code_ = static_cast<uint8_t>(sector_num);
	sd_num_ = static_cast<uint8_t>(num);
	sector_pars_ = TConfig::GetSectorPars(tme_code_);
	
	for(uint8_t i = 0; i < sector_pars_.tme_max_num_in_page; i++) {
		if (*reinterpret_cast<uint32_t*>(&buf[ i * sector_pars_.tme_size ]) == tme_time) {
			return i;
		}
	}

	return -1;
}

template <typename TBoard, typename TPoolAllocatorPort, typename TConfig>
template <board::Sd num, auto sector_num>
int32_t DataStorage<TBoard, TPoolAllocatorPort, TConfig>::ReadTmeBunch(uint32_t tme_time, uint32_t tme_step, uint32_t tme_qty, uint8_t *tmes_data) {
	assert(static_cast<uint8_t>(sector_num) < TConfig::GetSectorsCount());
	assert(tme_qty > 0);
	assert(tme_step >= 1);

	tme_code_ = static_cast<uint8_t>(sector_num);
	sd_num_ = static_cast<uint8_t>(num);
	sector_pars_ = TConfig::GetSectorPars(tme_code_);
	
	memset(tmes_data, FREE_BYTE, tme_qty * sector_pars_.tme_size);

	uint32_t newest_time;
	uint32_t oldest_time;
	
	err_code_ = FindNewestTime<num, sector_num>(&newest_time);
	if (err_code_ != ERROR_CODE_OK) {
		return err_code_;
	}
	err_code_ = FindOldestTime<num, sector_num>(&oldest_time);
	if (err_code_ != ERROR_CODE_OK) {
		return err_code_;
	}
	if (tme_time > newest_time) {
		return ERROR_CODE_GREATER_THEN_NEWEST_TIME;
	}
	if (tme_time + tme_step * tme_qty < oldest_time) {
		return ERROR_CODE_LESS_THEN_OLDEST_TIME;
	}

	RAIIAllocatedBuffer<uint8_t,TPoolAllocatorPort> buf(BUFFER_SIZE);

	enum alg_enum { read_tme_by_time, read_tme_by_addr_and_num };
	
	alg_enum alg = read_tme_by_time;
	bool flag_to_read_page = false;
	uint32_t tme_num_in_page_first;
	uint32_t page_to_read_first;
	uint32_t page_to_read_prev;
	uint32_t tme_time_read;

	for(uint8_t i = 0; i < tme_qty; i++) {
		switch (alg) {
			case read_tme_by_addr_and_num: {
				tme_num_in_page_ = tme_num_in_page_first % (uint32_t)sector_pars_.tme_max_num_in_page;
				page_to_read_ = page_to_read_first + tme_num_in_page_first / (uint32_t)sector_pars_.tme_max_num_in_page;

				while (page_to_read_ > sector_pars_.tme_range_start + sector_pars_.tme_range_length - 1) {
					page_to_read_ -= sector_pars_.tme_range_length;
				}
				
				flag_to_read_page = (page_to_read_ != page_to_read_prev) ? true : false;
				err_code_ = (flag_to_read_page == true) ? ReadPageWithAddrToBuf<num>(page_to_read_, buf.data) : ERROR_CODE_OK;
				if (err_code_ != ERROR_CODE_OK) {
					return err_code_;
				}
				
				if (*reinterpret_cast<uint32_t*>(&buf.data[ tme_num_in_page_ * sector_pars_.tme_size ]) == tme_time) {
					memcpy((void*)&tmes_data[ i * sector_pars_.tme_size ], (void*)&buf.data[ tme_num_in_page_ * sector_pars_.tme_size ], sector_pars_.tme_size);
					break; // break from case, next iterration is read_tme_by_addr_and_num
				} else {
					alg = read_tme_by_time; // go further to read_tme_by_time
				}
			}
				
			case read_tme_by_time: {				
				err_code_ = FindPageToRead<num,sector_num>(tme_time, &page_to_read_);
				if (err_code_ != ERROR_CODE_OK && 
					err_code_ != ERROR_CODE_TME_NOT_FOUND && 
					err_code_ != ERROR_CODE_LESS_THEN_OLDEST_TIME &&
					err_code_ != ERROR_CODE_GREATER_THEN_NEWEST_TIME
				) {
					return err_code_;
				}
				if (err_code_ == ERROR_CODE_TME_NOT_FOUND || 
					err_code_ == ERROR_CODE_LESS_THEN_OLDEST_TIME ||
					err_code_ == ERROR_CODE_GREATER_THEN_NEWEST_TIME
				) {
					break; // no such tme_time on sd
				}
				err_code_ = ReadPageWithAddrToBuf<num>(page_to_read_, buf.data);
				if (err_code_ != ERROR_CODE_OK) {
					return err_code_;
				}
				tme_num_in_page_ = FindNumInBufByTime<num,sector_num>(buf.data, tme_time);
				
				memcpy(
					(void*)&tmes_data[ i * sector_pars_.tme_size ],
					(void*)&buf.data[ tme_num_in_page_ * sector_pars_.tme_size ], 
					sector_pars_.tme_size
				);

				tme_num_in_page_first = tme_num_in_page_;
				page_to_read_first = page_to_read_;

				if (err_code_ == ERROR_CODE_OK) {
					alg = read_tme_by_addr_and_num;
				}
			}
			break;			
		}

		page_to_read_prev = page_to_read_;
		tme_num_in_page_first += tme_step;
		tme_time += tme_step;

		if (tme_time > newest_time) {
			break;
		}
	}
		
	return ERROR_CODE_OK;
}

}