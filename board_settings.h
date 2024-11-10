#pragma once

#include <cstdint>

namespace board {

enum class Uart { kDebug };	
enum class I2c { kInt, kExt };	
enum class Spi { kSd };

static constexpr uint8_t SD1_NUM = 0;
static constexpr uint8_t SD1_PORT = 0xE;
static constexpr uint8_t SD1_PIN = 7;
static constexpr uint8_t SD1_GPIO_PORT = 7;
static constexpr uint8_t SD1_GPIO_PIN = 7;

static constexpr uint8_t SD2_NUM = 1;
static constexpr uint8_t SD2_PORT = 0xE;
static constexpr uint8_t SD2_PIN = 8;
static constexpr uint8_t SD2_GPIO_PORT = 7;
static constexpr uint8_t SD2_GPIO_PIN = 8;

static constexpr uint8_t LPC3_PORT = 0x8;
static constexpr uint8_t LPC3_PIN = 7;
static constexpr uint8_t LPC3_GPIO_PORT = 4;
static constexpr uint8_t LPC3_GPIO_PIN = 7;

enum class PcaAddr { A0 = 0x26 };
enum class MpuAddr { A0 = 0x68, A1 = 0x69 };

enum class PcaPin { kMpu0 = 2, kMpu1 = 3, kSd0 = 4, kSd1 = 5 };

static constexpr uint8_t PCA_DIR = 0x01;
static constexpr uint8_t PCA_POL = 0x0;
static constexpr uint8_t PCA_STATE = 0xFE;

static constexpr uint32_t SPI_SD_BAUDRATE = 12500000;
static constexpr uint32_t UART_DEBUG_BAUDRATE = 115200;
static constexpr uint32_t I2C_INT_SPEED = 400000;
static constexpr uint32_t I2C_EXT_SPEED = 400000;

}