#pragma once

#include <cstdint>

namespace board {
	enum class Uart { kDebug };	
	enum class I2c { kInt, kExt };	
	enum class Spi { kSd };
	
	static constexpr uint8_t SD_COUNT = 2;
	static constexpr uint8_t MAGN_COUNT = 2;
	
	static constexpr uint8_t SD1_NUM = 0;
	static constexpr uint8_t SD1_PORT = 7;
	static constexpr uint8_t SD1_PIN = 7;
	
	static constexpr uint8_t SD2_NUM = 1;
	static constexpr uint8_t SD2_PORT = 7;
	static constexpr uint8_t SD2_PIN = 8;
	
	enum class PcaAddr { A0 = 0x26 };
	enum class MpuAddr { A0 = 0x68, A1 = 0x69 };
	
	enum class PcaPin { kMpu0 = 2, kMpu1 = 3, kSd0 = 4, kSd1 = 5 };
}