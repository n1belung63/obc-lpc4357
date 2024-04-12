#pragma once

#include "rtosdefs.h"

#include <limits>

#include <cstdint>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"
#include "portmacro.h"
#include "FreeRTOSConfig.h"

namespace wrtos {
class RtosWrapper {
public:
	template<typename Rtos, typename Task>
	inline static void CreateTask(
		Task &task, 
		const char *pName,
		TaskPriority prior,
		const std::uint16_t stackDepth,
		tStack *pStack
	) {	
		task.handle =  (
			xTaskCreate(
				static_cast<TaskFunction_t>(Rtos::Run),
				pName, 
				stackDepth, 
				&task, 
				static_cast<std::uint32_t>(prior),
				&task.handle
			) == pdTRUE) ?  task.handle : nullptr ;
	}

	inline static void wStart() {
		vTaskStartScheduler();
	}

	inline static void wSleep(const tTime timeOut) {
		vTaskDelay(timeOut);
	}

	inline static void wEnterCriticalSection() {
		taskENTER_CRITICAL();
	}

	inline static void wLeaveCriticalSection() {
		taskEXIT_CRITICAL();
	}
	
	inline static void wTaskSuspend(tTaskHandle const &taskHandle) {
		vTaskSuspend(taskHandle);
	}
	
	inline static void wTaskResume(tTaskHandle const &taskHandle) {
		vTaskResume(taskHandle);
	}

	inline static tMutexHandle wCreateMutex(tMutex &mutex) {
		return xSemaphoreCreateMutex();
	}

	inline static void wDeleteMutex(tMutexHandle &handle) {
		vSemaphoreDelete(handle);
	}

	inline static bool wLockMutex(tMutexHandle const &handle, tTime timeOut) {
		return static_cast<bool>(xSemaphoreTake(handle, timeOut));
	}

	inline static void wUnLockMutex(tMutexHandle const &handle) {
		xSemaphoreGiveFromISR(handle, nullptr);
	}

	inline static void wSleepUntil(tTime &last, const tTime timeOut) {
		vTaskDelayUntil(&last, timeOut);
	}

	inline static tTime wGetTicks() {
		return xTaskGetTickCount();
	}
	
	inline static tSemaphoreHandle wCreateSemaphore() {
		return xSemaphoreCreateBinary();
	}
	
	inline static void wDeleteSemaphore(tSemaphoreHandle &handle) {
		vSemaphoreDelete(handle);
	}
	
	inline static bool wGiveSemaphore(tSemaphoreHandle &handle) {
		return (xSemaphoreGiveFromISR(handle, nullptr) == pdTRUE);
	}
	
	inline static bool wTakeSemaphore(tSemaphoreHandle &handle, tTime timeOut) {
		return (xSemaphoreTake(handle, timeOut) == pdTRUE);
	}
	
} ;
} 