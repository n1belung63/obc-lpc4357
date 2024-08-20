#include "mock_board.h"

#include <gtest/gtest.h>

// #include "rtos_wrapper/rtos.h"
// #include "tasks/data_acquisition_task.h"
// #include "tasks/uart_task.h"

#include "../application/data_storage.h"

volatile void _delay_ms(uint32_t delay);

// using namespace wrtos;
using namespace board;
using namespace app;

uint8_t buf_512[512] = {0};	//todo: to refactor static allocator???
uint32_t buf_20[20] = {0};	//todo: to refactor static allocator???

volatile void _delay_ms(uint32_t delay) {
	static_cast<void>(0);
}

class DataStorageTest : public testing::Test {
protected:
    DataStorageTest() {
        // do stuff here
    }

  // ~DataStorageTest() override = default;

};

TEST_F(DataStorageTest, AddTmeToSd) {
    MockBoard& obc = MockBoard::GetInstance();
    DataStorage<MockBoard>& data_storage = DataStorage<MockBoard>::GetInstance();
    
    EXPECT_EQ(0, 0);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}