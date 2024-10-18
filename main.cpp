#include "board.h"

#include "rtos_wrapper/rtos.h"
#include "tasks/data_acquisition_task.h"
#include "tasks/uart_task.h"

#include "application/data_storage.h"
#include "system_abstraction/data_storage_config.h"
#include "pool-allocator/pool_allocator.h"
#include "pool_allocator_port.h"

volatile void _delay_ms(uint32_t delay);

using namespace wrtos;
using namespace board;
using namespace app;
using namespace allocator;

volatile void _delay_ms(uint32_t delay) {
	volatile uint32_t cyc = delay * ((SystemCoreClock / 1000) / 6);	// maximum delay is 14.3165 s

	__asm volatile (
		"1:        		        \n"
		"	 ldr r1,%[cyc]      \n"		// 2 Cycles
		"	 subs r1,r1,#0x01   \n"		// 1 Cycle
		"	 str r1,%[cyc]      \n"		// 2 Cycles
		"	 bne 1b     		    \n"	  // 1 or 1 + P Cycles
		:													  // no output operand
		:  [cyc] "g" (cyc)					// input operand
		:  "r1"										  // clobbered register
	);
}

template<typename TPoolAllocatorPort>
using DefaultAllocator = PoolAllocator<DefaultBlockSize,DefaultBlockCount,TPoolAllocatorPort>;

int main(void) {
	DefaultAllocator<PoolAllocatorPort>& allocator = DefaultAllocator<PoolAllocatorPort>::GetInstance();
	
	Board& obc = Board::GetInstance();
	DataStorage<Board,PoolAllocatorPort,app::DataStorageConfig>& data_storage = DataStorage<Board,PoolAllocatorPort,app::DataStorageConfig>::GetInstance();
		
	DataAcquisitionTask<Board> data_acquisition_task;
	UartTask<Board> uart_task;
	
	Rtos::CreateTask(data_acquisition_task, "DataAcqTask", TaskPriority::aboveNormal);
	Rtos::CreateTask(uart_task, "UartTask", TaskPriority::highest);
	Rtos::Start();
	
	return 1;
}
