#include "mock_board.h"

#include <cassert>
#include <fstream>
#include <iostream>

using namespace board;
using namespace std;

extern volatile void _delay_ms(uint32_t delay);

MockBoard::MockBoard() {	
	status_pool_.sd[static_cast<uint8_t>(Sd::kNum1)] = Status::kWorked;
	status_pool_.sd[static_cast<uint8_t>(Sd::kNum2)] = Status::kWorked;
	status_pool_.mpu[static_cast<uint8_t>(Magn::kNum1)] = Status::kWorked;
	status_pool_.mpu[static_cast<uint8_t>(Magn::kNum2)] = Status::kWorked;
}

int32_t MockBoard::SdPageWrite(Sd num, uint32_t page_addr, uint8_t page[board::SD_PAGE_SIZE]) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);

	auto write_single_block = [](Sd num, uint32_t page_addr, uint8_t page[board::SD_PAGE_SIZE]) {
		string file_name = "pages\\sd_"s + to_string(static_cast<uint8_t>(num)) + "_page_" + to_string(page_addr) + ".dat"s;
		ofstream file(file_name, ios::out | ios::binary);	
		if (file.is_open()) {
			file.write((char*)&page[0], SD_PAGE_SIZE);
		}
		file.close();
	};

	if (status_pool_.sd[static_cast<uint8_t>(num)] == Status::kFailed) {
		return ERROR_CODE_NOT_INITED;
	}		
	write_single_block(num, page_addr, page);
	return ERROR_CODE_OK;
}

int32_t MockBoard::SdPageRead(Sd num, uint32_t page_addr, uint8_t page[board::SD_PAGE_SIZE]) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);

	auto read_single_block = [](Sd num, uint32_t page_addr, uint8_t page[SD_PAGE_SIZE]) {
		string file_name = "pages\\sd_"s + to_string(static_cast<uint8_t>(num)) + "_page_" + to_string(page_addr) + ".dat"s;
		ifstream file(file_name, ios::out | ios::binary);	
		if (file.is_open()) {
			file.read((char*)&page[0], SD_PAGE_SIZE);
		}
		file.close();
	};

	if (status_pool_.sd[static_cast<uint8_t>(num)] == Status::kFailed) {
		return ERROR_CODE_NOT_INITED;
	}		
	read_single_block(num, page_addr, page);
	return ERROR_CODE_OK;
}

int32_t MockBoard::SdBlock(Sd num) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);
	
	if (GetSdStatus(num) == Status::kWorked) {
		status_pool_.sd[static_cast<uint8_t>(num)] = Status::kBlocked;
	}
	return ERROR_CODE_OK;
}

int32_t MockBoard::SdUnblock(Sd num) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);
	
	if (GetSdStatus(num) == Status::kBlocked) {
		status_pool_.sd[static_cast<uint8_t>(num)] = Status::kWorked;
	}
	return ERROR_CODE_OK;
}

int32_t MockBoard::SdRangeErase(Sd num, uint32_t start_addr, uint32_t end_addr) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);
	return ERROR_CODE_OK;

	//todo: delete binary???
	
	// switch(num) {
	// 	case Sd::kNum1: {
	// 		if (status_pool_.sd[static_cast<uint8_t>(Sd::kNum1)] == Status::kFailed) {
	// 			return ERROR_CODE_NOT_INITED;
	// 		}					
	// 		Sd0& sd0 = Sd0::GetInstance();
	// 		return sd0.Erase(start_addr, end_addr);
	// 	}
			
	// 	case Sd::kNum2: {
	// 		if (status_pool_.sd[static_cast<uint8_t>(Sd::kNum2)] == Status::kFailed) {
	// 			return ERROR_CODE_NOT_INITED;
	// 		}	
	// 		Sd1& sd1 = Sd1::GetInstance();
	// 		return sd1.Erase(start_addr, end_addr);
	// 	}
	// }
}

int32_t MockBoard::MagnRead(Magn num, MagnData& data) {
	assert(num == Magn::kNum1 || num == Magn::kNum2);
	return ERROR_CODE_OK;

	//todo: create random data???
	
	// switch(num) {
	// 	case Magn::kNum1: {
	// 		if (status_pool_.mpu[static_cast<uint8_t>(Magn::kNum1)] == Status::kFailed) {
	// 			return ERROR_CODE_NOT_INITED;
	// 		}
			
	// 		Mpu0& mpu0 = Mpu0::GetInstance();
	// 		sensor::MPU9250_DATA_t mpu_data = {0};
	// 		int32_t res = mpu0.Read(&mpu_data);
	// 		if (res != 0) {
	// 			status_pool_.mpu[static_cast<uint8_t>(Magn::kNum1)] = Status::kFailed;
	// 			return res;
	// 		}
			
	// 		data.A_X = mpu_data.A_X;
	// 		data.A_Y = mpu_data.A_Y;
	// 		data.A_Z = mpu_data.A_Z;
	// 		data.B_X = mpu_data.B_X;
	// 		data.B_Y = mpu_data.B_Y;
	// 		data.B_Z = mpu_data.B_Z;
	// 		data.G_X = mpu_data.G_X;
	// 		data.G_Y = mpu_data.G_Y;
	// 		data.G_Z = mpu_data.G_Z;
	// 		data.T 	 = mpu_data.T;	
			
	// 		return res;
	// 	}
			
	// 	case Magn::kNum2: {
	// 		if (status_pool_.mpu[static_cast<uint8_t>(Magn::kNum2)] == Status::kFailed) {
	// 			return ERROR_CODE_NOT_INITED;
	// 		}
			
	// 		Mpu1& mpu1 = Mpu1::GetInstance();
	// 		sensor::MPU9250_DATA_t mpu_data = {0};
	// 		int32_t res = mpu1.Read(&mpu_data);
	// 		if (res != 0) {
	// 			status_pool_.mpu[static_cast<uint8_t>(Magn::kNum1)] = Status::kFailed;
	// 			return res;
	// 		}
			
	// 		data.A_X = mpu_data.A_X;
	// 		data.A_Y = mpu_data.A_Y;
	// 		data.A_Z = mpu_data.A_Z;
	// 		data.B_X = mpu_data.B_X;
	// 		data.B_Y = mpu_data.B_Y;
	// 		data.B_Z = mpu_data.B_Z;
	// 		data.G_X = mpu_data.G_X;
	// 		data.G_Y = mpu_data.G_Y;
	// 		data.G_Z = mpu_data.G_Z;
	// 		data.T 	 = mpu_data.T;	
			
	// 		return res;
	// 	}
	// }
}

Status MockBoard::GetMagnStatus(Magn num) {
	assert(num == Magn::kNum1 || num == Magn::kNum2);
	return status_pool_.mpu[static_cast<uint8_t>(num)];
}
Status MockBoard::GetSdStatus(Sd num) {
	assert(num == Sd::kNum1 || num == Sd::kNum2);
	return status_pool_.sd[static_cast<uint8_t>(num)];
}