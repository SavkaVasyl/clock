#include "ldr.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1;

#define LDR_READ_INTERVAL_MS    1000U
#define LDR_LIGHT_THRESHOLD     2100U
#define LDR_DARK_THRESHOLD      1900U
#define LDR_SAMPLES_COUNT       8U
#define LDR_ADC_TIMEOUT_MS      10U

static uint16_t ldrValue = 0U;
static uint32_t lastReadTick = 0U;
static uint8_t isLight = 1U;

static uint16_t LDR_Read(void) {
	uint32_t sum = 0U;
	for (uint8_t i = 0U; i < LDR_SAMPLES_COUNT; i++) {
		if (HAL_ADC_Start(&hadc1) != HAL_OK) {
			return ldrValue;
		}
		if (HAL_ADC_PollForConversion(&hadc1,
		LDR_ADC_TIMEOUT_MS) != HAL_OK) {
			HAL_ADC_Stop(&hadc1);

			return ldrValue;
		}
		sum += HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
	}
	return (uint16_t) (sum / LDR_SAMPLES_COUNT);
}

void LDR_Init(void) {
	ldrValue = 0U;
	isLight = 1U;
	lastReadTick = HAL_GetTick() - LDR_READ_INTERVAL_MS;
}

uint8_t LDR_Update(void) {
	uint32_t now;
	uint8_t previousState;
	now = HAL_GetTick();
	if ((uint32_t) (now - lastReadTick) < LDR_READ_INTERVAL_MS) {
		return 0U;
	}
	lastReadTick = now;
	ldrValue = LDR_Read();
	previousState = isLight;
	if (isLight != 0U) {
		if (ldrValue <= LDR_DARK_THRESHOLD) {
			isLight = 0U;
		}
	} else {
		if (ldrValue >= LDR_LIGHT_THRESHOLD) {
			isLight = 1U;
		}
	}
	return (isLight != previousState) ? 1U : 0U;
}

uint8_t LDR_IsLight(void) {
	return isLight;
}
