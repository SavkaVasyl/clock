
#include "buzzer.h"

#include "settings.h"
#include "main.h"

#include <stddef.h>

extern TIM_HandleTypeDef htim4;

#define BUZZER_SOUND_MS  150U
#define BUZZER_PAUSE_MS  200U

static uint8_t buzzerEnabled;
static volatile uint8_t signalsLeft;
static volatile uint8_t soundActive;
static volatile uint16_t timerMs;

static void Buzzer_StartTone(void) {
	HAL_TIM_PWM_Start(&htim4,
	TIM_CHANNEL_3);
}

static void Buzzer_StopTone(void) {
	HAL_TIM_PWM_Stop(&htim4,
	TIM_CHANNEL_3);
}

static void Buzzer_StartSignals(uint8_t count) {
	if ((buzzerEnabled == 0U) || (count == 0U) || (signalsLeft != 0U)) {
		return;
	}

	signalsLeft = count;
	soundActive = 1U;
	timerMs = BUZZER_SOUND_MS;

	Buzzer_StartTone();
}

void Buzzer_Init(void) {
	Buzzer_StopTone();
	buzzerEnabled = Settings_LoadBuzzer();
	signalsLeft = 0U;
	soundActive = 0U;
	timerMs = 0U;

	HAL_GPIO_WritePin(
	BUZZER_LED_GPIO_Port,
	BUZZER_LED_Pin, (buzzerEnabled != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Buzzer_Toggle(void) {
	buzzerEnabled ^= 1U;
	HAL_GPIO_WritePin(
	BUZZER_LED_GPIO_Port,
	BUZZER_LED_Pin, (buzzerEnabled != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	if (buzzerEnabled == 0U) {
		Buzzer_StopTone();
		signalsLeft = 0U;
		soundActive = 0U;
		timerMs = 0U;
	}
	Settings_SaveBuzzer(buzzerEnabled);
}

void Buzzer_CheckTime(const ClockTime_t *time) {
	if ((time == NULL) || (buzzerEnabled == 0U)) {
		return;
	}
	if ((time->hours > 23U) || (time->minutes > 59U) || (time->seconds > 59U)) {
		return;
	}
	if (time->seconds != 0U) {
		return;
	}

	switch (time->minutes) {
	case 0U:
		Buzzer_StartSignals(4U);
		break;
	case 15U:
		Buzzer_StartSignals(1U);
		break;
	case 30U:
		Buzzer_StartSignals(2U);
		break;
	case 45U:
		Buzzer_StartSignals(3U);
		break;
	default:
		break;
	}
}

void Buzzer_TimerCallback(void) {
	if (signalsLeft == 0U) {
		return;
	}
	if (timerMs > 0U) {
		timerMs--;

		if (timerMs > 0U) {
			return;
		}
	}
	if (soundActive != 0U) {
		Buzzer_StopTone();

		soundActive = 0U;
		signalsLeft--;
		if (signalsLeft == 0U) {
			return;
		}
		timerMs = BUZZER_PAUSE_MS;
		return;
	}

	Buzzer_StartTone();

	soundActive = 1U;
	timerMs = BUZZER_SOUND_MS;
}
