

#include "ds3231.h"
#include "stm32f4xx_hal.h"

#include <stddef.h>

extern I2C_HandleTypeDef hi2c1;

#define DS3231_ADDR            (0x68U << 1U)

#define DS3231_REG_SECONDS     0x00U
#define DS3231_REG_DATE        0x04U

#define DS3231_TIMEOUT_MS      20U

static uint8_t DS3231_BcdToDecimal(uint8_t value) {
	return (uint8_t) (((value >> 4U) * 10U) + (value & 0x0FU));
}

static uint8_t DS3231_DecimalToBcd(uint8_t value) {
	return (uint8_t) (((value / 10U) << 4U) | (value % 10U));
}

uint8_t DS3231_IsReady(void) {
	return (HAL_I2C_IsDeviceReady(&hi2c1,
	DS3231_ADDR, 3U,
	DS3231_TIMEOUT_MS) == HAL_OK) ? 1U : 0U;
}

uint8_t DS3231_ReadTime(ClockTime_t *time) {
	uint8_t data[3];
	if (time == NULL) {
		return 0U;
	}
	if (HAL_I2C_Mem_Read(&hi2c1,
	DS3231_ADDR,
	DS3231_REG_SECONDS,
	I2C_MEMADD_SIZE_8BIT, data, sizeof(data),
	DS3231_TIMEOUT_MS) != HAL_OK) {
		return 0U;
	}
	time->seconds = DS3231_BcdToDecimal(data[0] & 0x7FU);
	time->minutes = DS3231_BcdToDecimal(data[1] & 0x7FU);
	time->hours = DS3231_BcdToDecimal(data[2] & 0x3FU);
	if ((time->hours > 23U) || (time->minutes > 59U) || (time->seconds > 59U)) {
		return 0U;
	}
	return 1U;
}

uint8_t DS3231_SetTime(const ClockTime_t *time) {
	uint8_t data[3];
	if (time == NULL) {
		return 0U;
	}
	if ((time->hours > 23U) || (time->minutes > 59U) || (time->seconds > 59U)) {
		return 0U;
	}
	data[0] = DS3231_DecimalToBcd(time->seconds);
	data[1] = DS3231_DecimalToBcd(time->minutes);
	data[2] = DS3231_DecimalToBcd(time->hours);
	if (HAL_I2C_Mem_Write(&hi2c1,
	DS3231_ADDR,
	DS3231_REG_SECONDS,
	I2C_MEMADD_SIZE_8BIT, data, sizeof(data),
	DS3231_TIMEOUT_MS) != HAL_OK) {
		return 0U;
	}
	return 1U;
}

uint8_t DS3231_ReadDate(ClockDate_t *date) {
	uint8_t data[3];
	if (date == NULL) {
		return 0U;
	}
	if (HAL_I2C_Mem_Read(&hi2c1,
	DS3231_ADDR,
	DS3231_REG_DATE,
	I2C_MEMADD_SIZE_8BIT, data, sizeof(data),
	DS3231_TIMEOUT_MS) != HAL_OK) {
		return 0U;
	}
	date->day = DS3231_BcdToDecimal(data[0] & 0x3FU);
	date->month = DS3231_BcdToDecimal(data[1] & 0x1FU);
	date->year = (uint16_t) (2000U + DS3231_BcdToDecimal(data[2]));
	if ((date->day < 1U) || (date->day > 31U) || (date->month < 1U)
			|| (date->month > 12U) || (date->year < 2000U)
			|| (date->year > 2099U)) {
		return 0U;
	}
	return 1U;
}

uint8_t DS3231_SetDate(const ClockDate_t *date) {
	uint8_t data[3];
	if (date == NULL) {
		return 0U;
	}
	if ((date->day < 1U) || (date->day > 31U) || (date->month < 1U)
			|| (date->month > 12U) || (date->year < 2000U)
			|| (date->year > 2099U)) {
		return 0U;
	}
	data[0] = DS3231_DecimalToBcd(date->day);
	data[1] = DS3231_DecimalToBcd(date->month);
	data[2] = DS3231_DecimalToBcd((uint8_t) (date->year - 2000U));
	if (HAL_I2C_Mem_Write(&hi2c1,
	DS3231_ADDR,
	DS3231_REG_DATE,
	I2C_MEMADD_SIZE_8BIT, data, sizeof(data),
	DS3231_TIMEOUT_MS) != HAL_OK) {
		return 0U;
	}
	return 1U;
}
