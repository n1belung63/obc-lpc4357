#pragma once

#include "singelton/singelton.h"
#include "wrappers/i2c_wrapper.h"
#include "board_settings.h"

#include <cstdint>
#include <cstring>

extern volatile void _delay_ms(uint32_t delay);

namespace sensor {
	
typedef struct {
	int16_t B_X;
	int16_t B_Y;
	int16_t B_Z;
	int16_t G_X;
	int16_t G_Y;
	int16_t G_Z;
	int16_t A_X;
	int16_t A_Y;
	int16_t A_Z;
	uint16_t T;
} MPU9250_DATA_t;

template <board::MpuAddr addr>
class Mpu9250 : public Singleton<Mpu9250<addr>> {
	friend class Singleton<Mpu9250<addr>>;
public:
	static constexpr int32_t ERROR_CODE_OK = 0;

	int32_t Read(MPU9250_DATA_t *data);

	int32_t GetLastErrorCode();
private:
	static constexpr uint8_t MAG_I2C_ADDR = 0x0C;
	static constexpr uint8_t I2C_READ_FLAG = 0x80;

	static constexpr uint8_t REG_MAG_CNTL1 = 0x0A;
	static constexpr uint8_t REG_MAG_CNTL2 = 0x0B;

	static constexpr uint8_t MASK_MAG_CNTL1_CONT_MEAS_MODE_16_BIT = (2 | (1<<4));
	static constexpr uint8_t MASK_MAG_CNTL2_RESET = (1<<0);

	static constexpr uint8_t REG_SMPLRT_DIV = 0x19;
	static constexpr uint8_t REG_CONFIG = 0x1A;
	static constexpr uint8_t REG_GYRO_CONFIG = 0x1B;
	static constexpr uint8_t REG_ACCEL_CONFIG = 0x1C;

	static constexpr uint8_t MASK_CONFIG_250_HZ = 0x06;

	static constexpr uint32_t INTERNAL_SAMPLE_RATE = 1000;
	static constexpr uint32_t SAMPLE_RATE_HZ = 50;
	
	static constexpr uint8_t MASK_SMPLRT_DIV = INTERNAL_SAMPLE_RATE / SAMPLE_RATE_HZ - 1;

	static constexpr uint8_t GYRO_FULL_SCALE_250DPS = (0<<3);
	static constexpr uint8_t GYRO_FULL_SCALE_500DPS = (1<<3);
	static constexpr uint8_t GYRO_FULL_SCALE_1000DPS = (2<<3);
	static constexpr uint8_t GYRO_FULL_SCALE_2000DPS = (3<<3);

	static constexpr uint8_t ACCEL_FULL_SCALE_2G = (0<<3);
	static constexpr uint8_t ACCEL_FULL_SCALE_4G = (1<<3);
	static constexpr uint8_t ACCEL_FULL_SCALE_8G = (2<<3);
	static constexpr uint8_t ACCEL_FULL_SCALE_16G = (3<<3);

	static constexpr uint8_t MASK_GYRO_CONFIG = (0x0 | GYRO_FULL_SCALE_250DPS);
	static constexpr uint8_t MASK_ACCEL_CONFIG = ACCEL_FULL_SCALE_2G;

	static constexpr uint8_t REG_I2C_MST_CTRL = 0x24;
	static constexpr uint8_t REG_I2C_SLV0_ADDR = 0x25;
	static constexpr uint8_t REG_I2C_SLV0_REG = 0x26;
	static constexpr uint8_t REG_I2C_SLV0_CTRL = 0x27;
	static constexpr uint8_t MASK_I2C_SLV0_I2C_SLV0_EN = (1<<7);
	static constexpr uint8_t MASK_I2C_SLV0_CTRL = (MASK_I2C_SLV0_I2C_SLV0_EN | 7);
	static constexpr uint8_t MASK_I2C_MST_CTRL_400_KHZ = 0x0D;

	static constexpr uint8_t REG_I2C_SLV0_DO = 0x63;
	static constexpr uint8_t REG_USER_CTRL = 0x6A;
	static constexpr uint8_t MASK_USER_CTRL_I2C_MST_EN = (1<<5);
	
	static constexpr uint8_t REG_PWR_MGMT_1 = 0x6B;
	static constexpr uint8_t MASK_PWR_MGMT_1_H_RESET = (1<<7);
	static constexpr uint8_t MASK_PWR_MGMT_1_INT_OSC = (0<<0);
	static constexpr uint8_t MASK_PWR_MGMT_1_AUTO_SEL = (1<<0);

	static constexpr uint8_t REG_MAG_XOUT_L = 0x03;
	static constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;

	static constexpr uint8_t RESPONSE_SIZE = 20;
	
	Mpu9250();
	int32_t Init();
	int32_t Write(uint8_t reg, uint8_t buffer[], uint8_t count);
	
	inline __attribute__((always_inline)) void CombineRegAndData(uint8_t reg, uint8_t data, uint8_t msg[2]) {
		msg[0] = reg;
		msg[1] = data;
	}

	uint8_t addr_;
	using IntI2C = comm::I2c<board::I2c::kInt, board::I2C_INT_SPEED>;
	IntI2C& i2c_;
	int32_t err_code_;	
};

template <board::MpuAddr addr>
Mpu9250<addr>::Mpu9250() : addr_(static_cast<uint8_t>(addr)), i2c_(Singleton<IntI2C>::GetInstance()) {
	Init();
}

template <board::MpuAddr addr>
int32_t Mpu9250<addr>::Init() {
	uint8_t data_write[2] = {0};
		
	CombineRegAndData(REG_PWR_MGMT_1, MASK_PWR_MGMT_1_H_RESET, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	_delay_ms(500);
			
	CombineRegAndData(REG_PWR_MGMT_1, MASK_PWR_MGMT_1_INT_OSC, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	_delay_ms(150);
	
	CombineRegAndData(REG_PWR_MGMT_1, MASK_PWR_MGMT_1_AUTO_SEL, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	_delay_ms(100);
	
	CombineRegAndData(REG_GYRO_CONFIG, MASK_GYRO_CONFIG, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	_delay_ms(100);
	
	CombineRegAndData(REG_ACCEL_CONFIG, MASK_ACCEL_CONFIG, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	_delay_ms(30);
	
	CombineRegAndData(REG_CONFIG, MASK_CONFIG_250_HZ, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	_delay_ms(30);
	
	CombineRegAndData(REG_SMPLRT_DIV, MASK_SMPLRT_DIV, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	_delay_ms(30);
	
	CombineRegAndData(REG_USER_CTRL, MASK_USER_CTRL_I2C_MST_EN, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	_delay_ms(10);
	
	CombineRegAndData(REG_I2C_MST_CTRL, MASK_I2C_MST_CTRL_400_KHZ, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	_delay_ms(100);
			
	err_code_ = Write(REG_MAG_CNTL2, (uint8_t[1]){ MASK_MAG_CNTL2_RESET }, 1);
	if (err_code_ != ERROR_CODE_OK) return err_code_;
	_delay_ms(30);
	
	err_code_ = Write(REG_MAG_CNTL1, (uint8_t[1]){ MASK_MAG_CNTL1_CONT_MEAS_MODE_16_BIT }, 1);
	if (err_code_ != ERROR_CODE_OK) return err_code_;
	_delay_ms(30);
	
	err_code_ = ERROR_CODE_OK;
	return err_code_;
}

template <board::MpuAddr addr>
int32_t Mpu9250<addr>::Read(MPU9250_DATA_t* data) {
	uint8_t data_control;
	uint8_t data_write[2];
	uint8_t response[RESPONSE_SIZE];
	
	CombineRegAndData(REG_I2C_SLV0_ADDR, MAG_I2C_ADDR | I2C_READ_FLAG, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	
	CombineRegAndData(REG_I2C_SLV0_REG, REG_MAG_XOUT_L, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;

	CombineRegAndData(REG_I2C_SLV0_CTRL, MASK_I2C_SLV0_CTRL, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	
	data_control = REG_ACCEL_XOUT_H;
	err_code_ = i2c_.Write(addr_, (&data_control), 1);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;
	err_code_ = i2c_.Read(addr_, response, RESPONSE_SIZE);
	if (err_code_ != ARM_DRIVER_OK) return err_code_;

	data->A_X = ((int16_t)response[0]<<8)|response[1];
	data->A_Y = ((int16_t)response[2]<<8)|response[3];
	data->A_Z = ((int16_t)response[4]<<8)|response[5];
	
	data->T = ((int16_t)response[6]<<8)|response[7];
	
	data->G_X = ((int16_t)response[8]<<8)|response[9];
	data->G_Y = ((int16_t)response[10]<<8)|response[11];
	data->G_Z = ((int16_t)response[12]<<8)|response[13];
	
	data->B_X = ((int16_t)response[15]<<8)|response[14];
	data->B_Y = ((int16_t)response[17]<<8)|response[16];
	data->B_Z = ((int16_t)response[19]<<8)|response[18];

	err_code_ = ERROR_CODE_OK;
	return err_code_;
}


template <board::MpuAddr addr>
int32_t Mpu9250<addr>::Write(uint8_t reg, uint8_t buffer[], uint8_t count) {
	uint8_t data_write[2] = {0};

	CombineRegAndData(REG_I2C_SLV0_ADDR, MAG_I2C_ADDR, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK)
		return err_code_;
			
	CombineRegAndData(REG_I2C_SLV0_REG, reg, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK)
		return err_code_;
	
	CombineRegAndData(REG_I2C_SLV0_DO, *buffer, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK)
		return err_code_;
	
	CombineRegAndData(REG_I2C_SLV0_CTRL, MASK_I2C_SLV0_I2C_SLV0_EN | count, data_write);
	err_code_ = i2c_.Write(addr_, data_write, 2);
	if (err_code_ != ARM_DRIVER_OK)
		return err_code_;
	
	err_code_ = ERROR_CODE_OK;
	return err_code_;
}

template <board::MpuAddr addr>
int32_t Mpu9250<addr>::GetLastErrorCode() {
	return err_code_;
}

}