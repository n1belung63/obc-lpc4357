#pragma once

#include <cstdint>

namespace board {

enum class Uart { kDebug };	
enum class I2c { kInt, kExt };	
enum class Spi { kSd };

static constexpr uint8_t SD1_NUM = 0;
static constexpr uint8_t SD1_PORT = 7;
static constexpr uint8_t SD1_PIN = 7;

static constexpr uint8_t SD2_NUM = 1;
static constexpr uint8_t SD2_PORT = 7;
static constexpr uint8_t SD2_PIN = 8;

static constexpr uint8_t LPC3_PORT = 8;
static constexpr uint8_t LPC3_PIN = 7;
static constexpr uint8_t LPC3_GPIO_PORT = 4;
static constexpr uint8_t LPC3_GPIO_NUM = 7;

enum class PcaAddr { A0 = 0x26 };
enum class MpuAddr { A0 = 0x68, A1 = 0x69 };

enum class PcaPin { kMpu0 = 2, kMpu1 = 3, kSd0 = 4, kSd1 = 5 };

static constexpr uint8_t PCA_DIR = 0x01;
static constexpr uint8_t PCA_POL = 0x0;
static constexpr uint8_t PCA_STATE = 0xFE;

}