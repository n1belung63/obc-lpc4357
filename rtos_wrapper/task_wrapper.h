#pragma once

#include <array>
#include <chrono>
#include <cstdint>

#include "rtosdefs.h"
#include "freertos_wrapper.h"

namespace wrtos {
using namespace std::chrono_literals;
	
enum class StackDepth : std::uint16_t {
	minimal = 128U,
	medium = 256U,
	big = 512U,
	biggest = 1024U
};

class ITask {		
	friend class Rtos;  
	friend class RtosWrapper;
	
public:
	virtual void Execute() = 0 ;

	inline static void Sleep(const std::chrono::milliseconds timeOut = 1000ms) {      
		RtosWrapper::wSleep(std::chrono::duration_cast<TicksPerSecond>(timeOut).count());   
	};

	inline void SleepUntil(const std::chrono::milliseconds timeOut = 1000ms) {     
		RtosWrapper::wSleepUntil(lastWakeTime, std::chrono::duration_cast<TicksPerSecond>(timeOut).count());
	};
	
	inline void SuspendMe() {     
		RtosWrapper::wTaskSuspend(handle);
	};
	
	inline void ResumeMe() {     
		RtosWrapper::wTaskResume(handle);
	};

private:
	tTaskContext context;
	tTaskHandle handle =  nullptr;

	tTime lastWakeTime = 0U;

	inline void Run() {
		lastWakeTime = RtosWrapper::wGetTicks();
		Execute();
	}
};

template<std::size_t stackSize>
class Task: public ITask {  
	friend class Rtos;  

	static constexpr std::size_t stackDepth = stackSize;
	std::array <tStack, stackSize> stack;
};
};
