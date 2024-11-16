#include "rtos.h"
#include "board_settings.h"

using namespace wrtos;

uint32_t Rtos::scheduler_time_ = 0;

extern "C" {
	void vApplicationIdleHook( void ) {

	}
}