#include "rtos.h"
#include "board_settings.h"

using namespace wrtos;

uint32_t Rtos::cycle_count_ = 0;
uint32_t Rtos::scheduler_time_ = 0;

extern "C" {
	void vApplicationIdleHook( void ) {
		Rtos::IterrateCycleCount();
		if ( Rtos::GetCycleCount() % configDATA_ACQUISITION_PERIOD_MS == 0 ) {
			Rtos::IterrateSchedulerTime();
		}
	}
}