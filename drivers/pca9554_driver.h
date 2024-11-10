#pragma once

#include "singelton/singelton.h"
#include "wrappers/i2c_wrapper.h"
#include "board_settings.h"

#include <cstdint>
#include <cstring>

extern volatile void _delay_ms(uint32_t delay);

namespace chip {
	
template <board::PcaAddr addr, uint8_t dir, uint8_t pol, uint8_t state>
class Pca9554 : public Singleton<Pca9554<addr,dir,pol,state>> {
	friend class Singleton<Pca9554<addr,dir,pol,state>>;
public:
	static constexpr int32_t ERROR_CODE_OK = 0;

	int32_t PinSet(uint8_t pin);
	int32_t PinReset(uint8_t pin);

	int32_t GetLastErrorCode();

private:
	static constexpr uint8_t REG_INPUT_PORT = 0x00;
	static constexpr uint8_t REG_OUTPUT_PORT = 0x01;

	static constexpr uint8_t REG_POLARITY_INVERSION = 0x02;
	static constexpr uint8_t REG_CONFIGURATION = 0x03;

	Pca9554();
	int32_t Init();

	int32_t WriteRegister(uint8_t reg, uint8_t data);
	int32_t ReadRegister(uint8_t reg, uint8_t* data);

	uint8_t addr_;
	using IntI2C = comm::I2c<board::I2c::kInt, board::I2C_INT_SPEED>;
	IntI2C& i2c_;
	int32_t err_code_;
};

template <board::PcaAddr addr, uint8_t dir, uint8_t pol, uint8_t state>
Pca9554<addr,dir,pol,state>::Pca9554() : addr_(static_cast<uint8_t>(addr)), i2c_(Singleton<IntI2C>::GetInstance()) {
	Init();
}

template <board::PcaAddr addr, uint8_t dir, uint8_t pol, uint8_t state>
int32_t Pca9554<addr,dir,pol,state>::Init() {	
	err_code_ = WriteRegister(REG_CONFIGURATION, dir);
	if(err_code_) return err_code_;
	
	err_code_ = WriteRegister(REG_POLARITY_INVERSION, pol);
	if(err_code_) return err_code_;
	
	err_code_ = WriteRegister(REG_OUTPUT_PORT, state);
	if(err_code_) return err_code_;
	
	err_code_ = ERROR_CODE_OK;
	return err_code_;
}

template <board::PcaAddr addr, uint8_t dir, uint8_t pol, uint8_t state>
int32_t Pca9554<addr,dir,pol,state>::WriteRegister(uint8_t reg, uint8_t data) {
	assert(reg <= REG_CONFIGURATION);
	err_code_ = i2c_.Write(addr_, (uint8_t[2]){ reg, data }, 2);
	return err_code_;
}

template <board::PcaAddr addr, uint8_t dir, uint8_t pol, uint8_t state>
int32_t Pca9554<addr,dir,pol,state>::ReadRegister(uint8_t reg, uint8_t* data){
	assert(reg <= REG_CONFIGURATION);
		
	err_code_ = i2c_.Write(addr_, &reg, 1);
	if(err_code_) return err_code_;
	
	err_code_ = i2c_.Read(addr_, data, 1);
	if (err_code_) return err_code_;
	
	err_code_ = ERROR_CODE_OK;
	return err_code_;
}

template <board::PcaAddr addr, uint8_t dir, uint8_t pol, uint8_t state>
int32_t Pca9554<addr,dir,pol,state>::PinSet(uint8_t pin) {
	assert(pin < 8);
	
	uint8_t current_state = 0;

	err_code_ = ReadRegister(REG_OUTPUT_PORT, &current_state);
	if(err_code_) return err_code_;

	current_state |= (0x01<<pin);
	err_code_ = WriteRegister(REG_OUTPUT_PORT, current_state);
	if(err_code_) return err_code_;
	
	err_code_ = ERROR_CODE_OK;
	return err_code_;
}

template <board::PcaAddr addr, uint8_t dir, uint8_t pol, uint8_t state>
int32_t Pca9554<addr,dir,pol,state>::PinReset(uint8_t pin) {
	assert(pin < 8);
	
	uint8_t current_state = 0;

	err_code_ = ReadRegister(REG_OUTPUT_PORT, &current_state);
	if(err_code_) return err_code_;

	current_state &= ~(0x01<<pin);
	err_code_ = WriteRegister(REG_OUTPUT_PORT, current_state);
	if(err_code_) return err_code_;
	
	err_code_ = ReadRegister(REG_OUTPUT_PORT, &current_state);
	if(err_code_) return err_code_;
	
	err_code_ = ERROR_CODE_OK;
	return err_code_;
}

template <board::PcaAddr addr, uint8_t dir, uint8_t pol, uint8_t state>
int32_t Pca9554<addr,dir,pol,state>::GetLastErrorCode() {
	return err_code_;
}

}