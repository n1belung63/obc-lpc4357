#pragma once

#include <array>
#include <chrono>

#include "rtosdefs.h"
#include "freertos_wrapper.h"

namespace wrtos {
class Semaphore {
public:
	static tSemaphoreHandle & GetInstance() {
		static tSemaphoreHandle handle = RtosWrapper::wCreateSemaphore();
		return handle;
	}
	
	static bool Take(std::chrono::milliseconds timeOut) {
		return RtosWrapper::wTakeSemaphore(Semaphore::GetInstance(), std::chrono::duration_cast<TicksPerSecond>(timeOut).count());
	}

	static bool Give() {
		return RtosWrapper::wGiveSemaphore(Semaphore::GetInstance());
	}
};
}