#include "rtos.h"

using namespace wrtos;

extern "C" {
	void xPortRITimerHandler(void) {
		LPC_RITIMER->CTRL |= 1;	
		Rtos::HandleRiTimerInterrupt();
	}
}
