#pragma once

#include "freertos_wrapper.h"
#include "task_wrapper.h"
#include "semphr_wrapper.h"
#include "rtosdefs.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

namespace wrtos {
using namespace std::chrono_literals;
	
static uint32_t scheduler_time = 0;
	
class Rtos {
	friend class RtosWrapper;    
public:
	template<typename Task>
	inline static void CreateTask(
		Task& task,
		const char *pName, 
		TaskPriority prior = TaskPriority::normal
	) {
		return RtosWrapper::CreateTask<Rtos>(task, pName, prior, task.stackDepth, task.stack.data());
	}

	inline static void Start() {
		RtosWrapper::wStart();
	}
	
	inline static void EnterCriticalSection() {
		RtosWrapper::wEnterCriticalSection();
	}
		
	inline static void LeaveCriticalSection() {
		RtosWrapper::wLeaveCriticalSection();
	}
	
	void static HandleRiTimerInterrupt() {
		if (xTaskGetTickCountFromISR() % 250 == 0) {
			scheduler_time++;
		}
	}
	
	inline static uint32_t GetSchedulerTime() {
		return scheduler_time;
	}
	
	inline static uint32_t GetCurrentTick() {
		return RtosWrapper::wGetTicks();
	}

private:
	inline static void Run(void *pContext) {
		static_cast<ITask*>(pContext)->Run();
	}
};
	
extern "C" {
	void xPortRITimerHandler(void);
}

};