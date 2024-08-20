#include "board.h"

#include "rtos_wrapper/rtos.h"
#include "tasks/data_acquisition_task.h"
#include "tasks/uart_task.h"

#include "application/data_storage.h"

volatile void _delay_ms(uint32_t delay);

using namespace wrtos;
using namespace board;
using namespace app;

uint8_t buf_512[512] = {0};	//todo: to refactor static allocator???
uint32_t buf_20[20] = {0};	//todo: to refactor static allocator???

volatile void _delay_ms(uint32_t delay) {
	volatile uint32_t cyc = delay * ((SystemCoreClock / 1000) / 6);	// maximum delay is 14.3165 s

	__asm volatile (
		" while:        		 \n"
		"	ldr r12,%[cyc]     \n"		// 2 Cycles
		"	subs r12,r12,#0x01 \n"		// 1 Cycle
		"	str r12,%[cyc]     \n"		// 2 Cycles
		"	bne while     		 \n"		// 1 or 1 + P Cycles
		:														// no output operand
		: [cyc] "g" (cyc)						// input operand
	);
}

int main(void) {
	Board& obc = Board::GetInstance();
	DataStorage<Board>& data_storage = DataStorage<Board>::GetInstance();
		
	DataAcquisitionTask<Board> data_acquisition_task;
	UartTask<Board> uart_task;
	
	Rtos::CreateTask(data_acquisition_task, "DataAcqTask", TaskPriority::aboveNormal);
	Rtos::CreateTask(uart_task, "UartTask", TaskPriority::highest);
	Rtos::Start();
	
	return 1;
}
