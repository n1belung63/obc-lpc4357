#pragma once

#include <chrono>

#include "LPC43xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "FreeRTOSConfig.h"


namespace wrtos {
using namespace std::chrono_literals;
	
extern "C" void vPortSVCHandler(void);
extern "C" void xPortPendSVHandler(void);
extern "C" void xPortSysTickHandler(void);
extern "C" void xPortRITimerHandler(void);
	

enum class TaskPriority : std::uint8_t {
		clear = 0,
		lowest = 1,
		belowNormal = 2,
		normal = 3,
		aboveNormal = 4,
		highest = 5,
		priorityMax = 6
};

using tTaskContext = StaticTask_t;
using tTaskHandle = TaskHandle_t;
using tStack = StackType_t ;

using tTaskEventMask = std::uint32_t ;
using tTime = TickType_t ;

using tMutex = StaticSemaphore_t;
using tMutexHandle = SemaphoreHandle_t;

using tSemaphoreHandle = SemaphoreHandle_t;

using TicksPerSecond = std::chrono::duration<tTime , std::ratio<portTICK_PERIOD_MS, 1000>> ;

}
