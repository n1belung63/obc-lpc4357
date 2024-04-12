#pragma once

#include "../singelton.h"
#include "../wrappers/spi_wrapper.h"
#include "../board_settings.h"

#include <cstdint>
#include <cstring>

#define SD_ERROR_NOT_IN_IDLE_STATE		(ARM_DRIVER_ERROR_SPECIFIC - 21)
#define SD_ERROR_SD_NOT_READY					(ARM_DRIVER_ERROR_SPECIFIC - 22)
#define SD_ERROR_ERROR_IN_R7_RESP			(ARM_DRIVER_ERROR_SPECIFIC - 23)
#define SD_ERROR_WRONG_REQUEST_FORMAT	(ARM_DRIVER_ERROR_SPECIFIC - 24)
#define SD_ERROR_ERROR_WRITING_BLOCK	(ARM_DRIVER_ERROR_SPECIFIC - 25)
#define SD_ERROR_ERROR_READING_BLOCK	(ARM_DRIVER_ERROR_SPECIFIC - 26)
#define SD_ERROR_BUSY_SIGNAL_TIMEOUT	(ARM_DRIVER_ERROR_SPECIFIC - 27)
#define SD_ERROR_NO_RESP_AFTER_R1			(ARM_DRIVER_ERROR_SPECIFIC - 28)
#define SD_ERROR_ERROR_IN_TOKEN				(ARM_DRIVER_ERROR_SPECIFIC - 29)
#define SD_ERROR_DATA_ERROR						(ARM_DRIVER_ERROR_SPECIFIC - 30)

extern volatile void _delay_ms(uint32_t delay);

namespace memory {
	
typedef struct {
	uint8_t sd_i;
	uint8_t sd_ss_port;
	uint8_t sd_ss_pin;
} SD_t;

enum class SdResponse { R1, R1b, R2, R3, R7 };

inline static SD_t sd_arr[2] = {
	{ .sd_i = SAMSAT_SD1_NUM, .sd_ss_port = SAMSAT_SD1_PORT, .sd_ss_pin = SAMSAT_SD1_PIN },
	{ .sd_i = SAMSAT_SD2_NUM, .sd_ss_port = SAMSAT_SD2_PORT, .sd_ss_pin = SAMSAT_SD2_PIN }
};
	
template <board::Sd num>
class Sd : public Singleton<Sd<num>> {
	friend class Singleton<Sd<num>>;
public:
	static constexpr uint32_t BLOCK_LENGTH = 512;
	static constexpr int32_t SD_DRIVER_OK = 0;

	int32_t ReadSingleBlock(uint32_t addr, uint8_t buf[BLOCK_LENGTH]);
	int32_t WriteSingleBlock(uint32_t addr, uint8_t buf[BLOCK_LENGTH]);
private:
	static constexpr uint8_t TEMP0 = 0xFF;

	static constexpr uint8_t CMD0 = 0;
	static constexpr uint32_t CMD0_ARG = 0x00000000;
	static constexpr uint8_t CMD0_CRC = 0x94;

	static constexpr uint8_t CMD8 = 8;
	static constexpr uint32_t CMD8_ARG = 0x000001AA;
	static constexpr uint8_t CMD8_CRC = 0x86;

	static constexpr uint8_t CMD9 = 9;
	static constexpr uint32_t CMD9_ARG = 0x00000000;
	static constexpr uint8_t CMD9_CRC = 0x00;

	static constexpr uint8_t CMD10 = 10;
	static constexpr uint32_t CMD10_ARG = 0x00000000;
	static constexpr uint8_t CMD10_CRC = 0x00;

	static constexpr uint8_t CMD13 = 13;
	static constexpr uint32_t CMD13_ARG = 0x00000000;
	static constexpr uint8_t CMD13_CRC = 0x00;

	static constexpr uint8_t CMD17 = 17;
	static constexpr uint8_t CMD17_CRC = 0x00;

	static constexpr uint8_t CMD24 = 24;
	static constexpr uint8_t CMD24_CRC = 0x00;

	static constexpr uint8_t CMD55 = 55;
	static constexpr uint32_t CMD55_ARG = 0x00000000;
	static constexpr uint8_t CMD55_CRC = 0x00;

	static constexpr uint8_t CMD58 = 58;
	static constexpr uint32_t CMD58_ARG = 0x00000000;
	static constexpr uint8_t CMD58_CRC = 0x00;

	static constexpr uint8_t ACMD41 = 41;
	static constexpr uint32_t ACMD41_ARG = 0x40000000;
	static constexpr uint8_t ACMD41_CRC = 0x00;

	static constexpr uint8_t SD_IN_IDLE_STATE = 0x01;
	static constexpr uint8_t SD_READY = 0x00;
	static constexpr uint8_t SD_R1_NO_ERROR(uint8_t X) { return X < 0x02; }

	static constexpr uint16_t SD_MAX_WRITE_ATTEMPTS = 3907;

	static constexpr uint8_t CMD0_MAX_ATTEMPTS = 255;
	static constexpr uint8_t CMD55_MAX_ATTEMPTS = 255;

	static constexpr uint16_t SD_MAX_READ_ATTEMPTS = 1563;
	static constexpr uint8_t SD_READ_START_TOKEN = 0xFE;
	static constexpr uint8_t SD_INIT_CYCLES = 10;

	static constexpr uint8_t SD_START_BLOCK_TOKEN = 0xFE;
	static constexpr uint8_t SD_ERROR_TOKEN = 0x00;
	static constexpr uint8_t SD_NONE_TOKEN = 0xFF;

	static constexpr uint8_t SD_DATA_ACCEPTED = 0x05;
	static constexpr uint8_t SD_DATA_REJECTED_CRC = 0x0B;
	static constexpr uint8_t SD_DATA_REJECTED_WRITE = 0x0D;

	const Sd & operator=(const Sd &) = delete;
	Sd();
	int32_t Init();

	SD_t* Resolve();
	int32_t Select();
	int32_t Unselect();
	int32_t Command(uint8_t cmd, uint32_t arg, uint8_t crc);
	int32_t CommandWrapped(uint32_t cmd, uint32_t arg, uint32_t crc, SdResponse resformat, uint8_t *resp);
	int32_t ReadResponse(SdResponse format, uint8_t *resp);
	
	SD_t* sd_;
	using Spi = comm::Spi<board::Spi::kSd>;	
	Spi& spi_;
};


template <board::Sd num>
Sd<num>::Sd() : sd_(Resolve()), spi_(Singleton<Spi>::GetInstance()) {
	Sd<num>::Init();
}

template <board::Sd num>
SD_t* Sd<num>::Resolve() {
	if constexpr (num == board::Sd::kNum1) {
		return &sd_arr[SAMSAT_SD1_NUM];
	}
	if constexpr (num == board::Sd::kNum2) {
		return &sd_arr[SAMSAT_SD2_NUM];
	}
	return NULL;
}

template <board::Sd num>
int32_t Sd<num>::Init() {
	assert(sd_!=NULL);
	
	int32_t res;
	uint8_t r1, r37[5], cmdAttempts = 0;

	/* Run power up sequence */
	res = spi_.SlaveSelectDisableGpio(sd_->sd_ss_port, sd_->sd_ss_pin);
	if(res!=ARM_DRIVER_OK)
		return res;
	
	for(uint8_t i = 0; i < SD_INIT_CYCLES; i++) {
		res = spi_.Write(&TEMP0, sizeof(uint8_t));
		if(res!=ARM_DRIVER_OK)
			return res;
	}

	/* Go Idle state (CMD0) */
	res = CommandWrapped(CMD0, CMD0_ARG, CMD0_CRC, SdResponse::R1, &r1);
	if(res!=SD_DRIVER_OK) 
		return res;
	
	while(r1 != SD_IN_IDLE_STATE && ++cmdAttempts<CMD0_MAX_ATTEMPTS) {	
		res = CommandWrapped(CMD0, CMD0_ARG, CMD0_CRC, SdResponse::R1, &r1);
		if(res!=SD_DRIVER_OK)
			return res;
	}
	
	if(cmdAttempts == CMD0_MAX_ATTEMPTS)
		return SD_ERROR_NOT_IN_IDLE_STATE;
	
	_delay_ms(1);
	
	/* Send Interface Conditions (CDM8) */
	res = CommandWrapped(CMD8, CMD8_ARG, CMD8_CRC, SdResponse::R7, r37);
	if(res!=SD_DRIVER_OK)
		return res;
	
	if(r37[0] != SD_IN_IDLE_STATE)
		return SD_ERROR_NOT_IN_IDLE_STATE;
	
	if(r37[4] != 0xAA)
		return SD_ERROR_ERROR_IN_R7_RESP;

	cmdAttempts = 0;
	do {
		/* Send application command (CMD55) */
		res = CommandWrapped(CMD55, CMD55_ARG, CMD55_CRC, SdResponse::R1, &r1);
		if(res!=SD_DRIVER_OK)
			return res;
		
		if(SD_R1_NO_ERROR(r1)) {
			/* Send operating condition (ACMD41) */			
			res = CommandWrapped(ACMD41, ACMD41_ARG, ACMD41_CRC, SdResponse::R1, &r1);
			if(res!=SD_DRIVER_OK)
				return res;
		}
		_delay_ms(1);
	} while(r1 != SD_READY && ++cmdAttempts<CMD55_MAX_ATTEMPTS);

	if(cmdAttempts == CMD55_MAX_ATTEMPTS)
		return SD_ERROR_SD_NOT_READY;
	
	_delay_ms(1);
	
	/* Reads OCR from SD Card (CMD58) */
	res = CommandWrapped(CMD58, CMD58_ARG, CMD58_CRC, SdResponse::R3, r37);
	if(res!=SD_DRIVER_OK)
		return res;
	
	return SD_DRIVER_OK;

}

template <board::Sd num>
int32_t Sd<num>::ReadSingleBlock(uint32_t addr, uint8_t buf[BLOCK_LENGTH]) {
	assert(sd_!=NULL);
	
	uint8_t token = SD_NONE_TOKEN, res, resp1;
	uint16_t readAttempts = 0;
	uint8_t cmd;
	uint8_t crc;
	uint8_t temp;
	
	res = Select();
	if(res!=SD_DRIVER_OK)
		return res;
		
	res = Command(CMD17, addr, CMD17_CRC);
	if(res!=SD_DRIVER_OK)
		return res;
	
	res = ReadResponse(SdResponse::R1, &resp1);
	if(res!=SD_DRIVER_OK)
		return res;

	if(resp1 == SD_READY) {
		while(token == SD_NONE_TOKEN && ++readAttempts != SD_MAX_READ_ATTEMPTS) {
			res = spi_.Transfer(&TEMP0, &token, sizeof(uint8_t));
			if(res!=ARM_DRIVER_OK)
				return res;
		}
		
		switch(token) {
			case SD_START_BLOCK_TOKEN: {
				for(uint16_t i = 0; i < BLOCK_LENGTH; i++) {
					res = spi_.Transfer(&TEMP0, &buf[i], sizeof(uint8_t));
					if(res!=ARM_DRIVER_OK)
						return res;
				}

				res = spi_.Write(&TEMP0, sizeof(uint8_t));
				if(res!=ARM_DRIVER_OK)
					return res;
			
				res = spi_.Write(&TEMP0, sizeof(uint8_t));
				if(res!=ARM_DRIVER_OK)
					return res;
				
				res = Unselect();
				if(res!=SD_DRIVER_OK)
					return res;
				
				return SD_DRIVER_OK;
			}
			
			case SD_ERROR_TOKEN: {
				res = Unselect();
				if(res!=SD_DRIVER_OK)
					return res;			
				return SD_ERROR_BUSY_SIGNAL_TIMEOUT;
			}		
			
			case SD_NONE_TOKEN: {
				res = Unselect();
				if(res!=SD_DRIVER_OK)
					return res;				
				return SD_ERROR_NO_RESP_AFTER_R1;
			}			
			
			default: {
				res = Unselect();
				if(res!=SD_DRIVER_OK)
					return res;			
				return SD_ERROR_DATA_ERROR;
			}					
		}
	} else {
		res = Unselect();
		if(res!=SD_DRIVER_OK)
			return res;
		
		return SD_ERROR_ERROR_READING_BLOCK;
	}
}

template <board::Sd num>
int32_t Sd<num>::WriteSingleBlock(uint32_t addr, uint8_t buf[BLOCK_LENGTH]) {
	assert(sd_!=NULL);
	
	uint16_t writeAttempts = 0;
	uint8_t token = SD_NONE_TOKEN, res, resp1, temp = SD_START_BLOCK_TOKEN;

	res = Select();
	if(res!=SD_DRIVER_OK)
		return res;
	
	res = Command(CMD24, addr, CMD24_CRC);
	if(res!=SD_DRIVER_OK)
		return res;
	
	res = ReadResponse(SdResponse::R1, &resp1);
	if(res!=SD_DRIVER_OK)
		return res;
	
	if(resp1 == SD_READY) {
		res = spi_.Write(&temp, sizeof(uint8_t));
		if(res!=ARM_DRIVER_OK)
			return res;
		
		for(uint16_t i = 0; i < BLOCK_LENGTH; i++) {
			res = spi_.Write(&buf[i], sizeof(uint8_t));
			if(res!=ARM_DRIVER_OK)
				return res;
		}
		
		while(token == SD_NONE_TOKEN && ++writeAttempts != SD_MAX_WRITE_ATTEMPTS) {
			res = spi_.Transfer(&TEMP0, &token, sizeof(uint8_t));
			if(res!=ARM_DRIVER_OK)
				return res;
		}
		
		if(token == SD_NONE_TOKEN) {
			res = Unselect();
			if(res!=SD_DRIVER_OK)
				return res;	
			return SD_ERROR_NO_RESP_AFTER_R1;
		}			
			
		if((token & 0x1F) == SD_DATA_ACCEPTED) {
			writeAttempts = 0;			
			temp = SD_ERROR_TOKEN;
			
			while(temp == SD_ERROR_TOKEN && ++writeAttempts < SD_MAX_WRITE_ATTEMPTS) {
				res = spi_.Transfer(&TEMP0, &temp, sizeof(uint8_t));
				if(res!=ARM_DRIVER_OK)
					return res;
			}
			
			if (writeAttempts == SD_MAX_WRITE_ATTEMPTS) {
				res = Unselect();
				if(res!=SD_DRIVER_OK)
					return res;						
				return SD_ERROR_BUSY_SIGNAL_TIMEOUT;			
			} else {
				res = Unselect();
				if(res!=0)
					return res;		
				return SD_DRIVER_OK;
			}
		} else {
			res = Unselect();
			if(res!=SD_DRIVER_OK)
				return res;		
			return SD_ERROR_ERROR_IN_TOKEN;
		}
	} else {
		res = Unselect();
		if(res!=SD_DRIVER_OK)
			return res;
		return SD_ERROR_ERROR_WRITING_BLOCK;
	}
}

template <board::Sd num>
int32_t Sd<num>::Select() {	
	int32_t res;
	
	res = spi_.Write(&TEMP0, sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
	
	res = spi_.SlaveSelectEnableGpio(sd_->sd_ss_port, sd_->sd_ss_pin);
	if(res!=ARM_DRIVER_OK)
		return res;
	
	res = spi_.Write(&TEMP0, sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
	
	return SD_DRIVER_OK;
}

template <board::Sd num>
int32_t Sd<num>::Unselect() {	
	int32_t res;
	
	res = spi_.Write(&TEMP0, sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
	
	res = spi_.SlaveSelectDisableGpio(sd_->sd_ss_port, sd_->sd_ss_pin);
	if(res!=ARM_DRIVER_OK)
		return res;
	
	res = spi_.Write(&TEMP0, sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
	
	return SD_DRIVER_OK;
}

template <board::Sd num>
int32_t Sd<num>::ReadResponse(SdResponse format, uint8_t *resp) {	
	int32_t res;
	uint8_t attempts = 0;
	
	res = spi_.Transfer(&TEMP0, &resp[0], sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
	
	while(resp[0] == TEMP0 && ++attempts < 8) {
		res = spi_.Transfer(&TEMP0, &resp[0], sizeof(uint8_t));
		if(res!=ARM_DRIVER_OK)
			return res;
	}

	switch (format) {
		case SdResponse::R1: {
		} break;
		
		case SdResponse::R1b: {
		} break;
		
		case SdResponse::R2: {	
			res = spi_.Transfer(&TEMP0, &resp[1], sizeof(uint8_t));
			if(res!=ARM_DRIVER_OK)
				return res;
		} break;
		
		case SdResponse::R3: {
			res = spi_.Transfer(&TEMP0, &resp[1], 4*sizeof(uint8_t));
			if(res!=ARM_DRIVER_OK)
				return res;
		} break;
		
		case SdResponse::R7: {		
			res = spi_.Transfer(&TEMP0, &resp[1], 4*sizeof(uint8_t));
			if(res!=ARM_DRIVER_OK)
				return res;
		} break;
	}
	
	return SD_DRIVER_OK;
}

template <board::Sd num>
int32_t Sd<num>::Command(uint8_t cmd, uint32_t arg, uint8_t crc) {	
	uint8_t temp;
	uint8_t res;
	
	temp = cmd|0x40;
	res = spi_.Write(&temp, sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
		
	temp = (uint8_t)(arg >> 24);
	res = spi_.Write(&temp, sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
	
	temp = (uint8_t)(arg >> 16);
	res = spi_.Write(&temp, sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
	
	temp = (uint8_t)(arg >> 8);
	res = spi_.Write(&temp, sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
	
	temp = (uint8_t)(arg);
	res = spi_.Write(&temp, sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
	
	temp = crc|0x01;
	res = spi_.Write(&temp, sizeof(uint8_t));
	if(res!=ARM_DRIVER_OK)
		return res;
	
	return SD_DRIVER_OK;
}

template <board::Sd num>
int32_t Sd<num>::CommandWrapped(uint32_t cmd, uint32_t arg, uint32_t crc, SdResponse resformat, uint8_t *resp) {	
	int32_t res;
	uint8_t r1;
	
	res = Select();
	if(res!=SD_DRIVER_OK)
		return res;
	
	res = Command(cmd, arg, crc);
	if(res!=SD_DRIVER_OK)
		return res;
	
	res = ReadResponse(resformat, resp);
	if(res!=SD_DRIVER_OK)
		return res;
	
	res = Unselect();
	if(res!=SD_DRIVER_OK)
		return res;
	
	return SD_DRIVER_OK;
}

}