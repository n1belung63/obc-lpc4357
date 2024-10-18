# Модуль регистрации и хранения данных на SD карте
Данный модуль предназначен для записи, хранения и чтения данных с SD-карт. Данные пишутся непосредственно на страницы SD карты, каждый тип данных в свой сектор, структура которых описывается в файле data_storage_config.h.

## Основной функционал
Данный модуль позволяет:
- последовательно добавлять пакеты данных на SD карту:  
```cpp
template <board::Sd num, auto sector_num>
int32_t AddTmeToSd(uint8_t *tme_data);
```
- считывать пакеты с SD карты по метке времени tme_time:
```cpp
template <board::Sd num, auto sector_num>
int32_t ReadTmeByTime(uint32_t tme_time, uint8_t *tme_data);
```
- считывать пачку пакетов с SD карты с заданными начальной меткой времени tme_start_time, шагом tme_step и количеством tme_qty:
```cpp
template <board::Sd num, auto sector_num>
int32_t ReadTmeBunch(uint32_t tme_start_time, uint32_t tme_step, uint32_t tme_qty, uint8_t *tmes_data);
```

## Структура хранения и записи данных
Структура секторов задаётся в файле ```data_storage_config.h```, например:

```cpp
#pragma once

#include "application/data_storage_config_api.h"
#include "system_abstraction/board_api.h"
#include "board_settings.h"

#include <cstdint>
#include <cassert>
#include <array>

namespace app {
	
// Структура сектора ObcSensors
typedef struct __attribute__ ((__packed__)) {
	uint32_t time;
	board::MagnData sensors[board::MAGN_COUNT];
} TObcMagnTme;

struct DataStorageConfig {
private:
	static constexpr uint32_t SECTORS_COUNT_ = 1;

	static constexpr uint32_t SECTOR_OBC_SENSORS_RANGE_START_ = 0x00000000;
	static constexpr uint32_t SECTOR_OBC_SENSORS_RANGE_LENGTH_ = 0x00200000; // 1Gb
	static constexpr uint32_t SECTOR_OBC_SENSORS_SIZE_ = sizeof(TObcMagnTme);
	static constexpr uint32_t SECTOR_OBC_SENSORS_MAX_NUM_ = board::SD_PAGE_SIZE / SECTOR_OBC_SENSORS_SIZE_;

	static constexpr std::array<TDataStorageSectorPars, SECTORS_COUNT_> SECTORS_PARS_ = {
		{
			SECTOR_OBC_SENSORS_RANGE_START_,
			SECTOR_OBC_SENSORS_RANGE_LENGTH_,
			SECTOR_OBC_SENSORS_SIZE_,
			SECTOR_OBC_SENSORS_MAX_NUM_,
		}
	};

public:
	static constexpr  TDataStorageSectorPars GetSectorPars(uint8_t sector_num) {
		assert(sector_num < SECTORS_COUNT_);
		return SECTORS_PARS_.at(sector_num);
	}
    
	static constexpr uint32_t GetSectorsCount() {
		return SECTORS_COUNT_;
	}

    // Перечисленеи с номерами секторов
	enum class Sector {
		ObcSensors = 0x00,
	};
};

}
```

Здесь:
```cpp
typedef struct {
	uint32_t tme_range_start;       // начало сектора с пакетами данных
	uint32_t tme_range_length;      // длина сектора с пакетами данных
	uint16_t tme_size;              // размер одного пакета данных
	uint16_t tme_max_num_in_page;   // целое число пакетов на одной странице
} TDataStorageSectorPars;
```

В границах своего сектора пакеты с данными пишутся последовательно. При достижении конца сектора запись начинается сначала. Соответственно, существует два варианта заполнения сектора: 
1) конец сектора не достигнут ни разу (рисунок 1); 
2) конец сектора был достигнут минимум один раз (рисунок 2).

| ![Alt text here](../images/case-1-and-2.svg) | 
|:--:| 
| *Рисунок 1 - Вид сектора, когда его конец не достигнут* |

| ![Alt text here](../images/case-3-and-4.svg) | 
|:--:| 
| *Рисунок 2 - Вид сектора, когда запись началась сначала, после достижения его конца* |
