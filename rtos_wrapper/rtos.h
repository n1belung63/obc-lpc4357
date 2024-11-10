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
		
class Rtos {
	friend class RtosWrapper;		
public:
	template<typename Task>
	inline static void CreateTask(
		Task& task,
		const char *pName, 
		TaskPriority prior = TaskPriority::normal
	) {
		return RtosWrapper::wCreateTask<Rtos>(task, pName, prior, task.stackDepth);
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
	
	inline static uint32_t GetCurrentTick() {
		return RtosWrapper::wGetTicks();
	}
	
	inline static void IterrateCycleCount() {
		cycle_count_++;
	}
	
	inline static uint32_t GetCycleCount() {
		return cycle_count_;
	}
	
	inline static void IterrateSchedulerTime() {
		scheduler_time_++;
	}
	
	inline static uint32_t GetSchedulerTime() {
		return scheduler_time_;
	}
	
	inline static void SetSchedulerTime(uint32_t time) {
		scheduler_time_ = time;
	}

private:
	inline static void Run(void *pContext) {
		static_cast<ITask*>(pContext)->Run();
	}

	static uint32_t cycle_count_;;
	static uint32_t scheduler_time_;
};

};