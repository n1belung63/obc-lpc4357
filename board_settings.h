#pragma once

namespace board {
	enum class Uart { kDebug };	
	enum class I2c { kInt, kExt };	
	enum class Spi { kSd };
	
	#define SAMSAT_SD1_NUM 0
	#define SAMSAT_SD1_PORT 7
	#define SAMSAT_SD1_PIN 7

	#define SAMSAT_SD2_NUM 1
	#define SAMSAT_SD2_PORT 7
	#define SAMSAT_SD2_PIN 8
	
	enum class Sd { kNum1, kNum2 };
	
	enum class PcaAddr { A0 = 0x26 };
	enum class PcaPin { kMpu0 = 2, kMpu1 = 3, kSd0 = 4, kSd1 = 5 };
	
	enum class MpuAddr { A0 = 0x68, A1 = 0x69 };
}