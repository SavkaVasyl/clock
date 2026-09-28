#include "clock.h"

#include "ds3231.h"
#include "display.h"
#include "hc595_leds.h"
#include "buzzer.h"
#include "dht11.h"
#include "ldr.h"
#include "settings.h"
#include "main.h"

#define RTC_READ_INTERVAL_MS  200U

typedef enum {
	CLOCK_MODE_RUN = 0,
	CLOCK_MODE_SET_HOURS,
	CLOCK_MODE_SET_MINUTES,
	CLOCK_MODE_SET_DAY,
	CLOCK_MODE_SET_MONTH,
	CLOCK_MODE_SET_YEAR,
	CLOCK_MODE_SET_BRIGHTNESS
} ClockMode_t;

static ClockTime_t currentTime = { .hours = 0U, .minutes = 0U, .seconds = 0U };

static ClockTime_t editedTime = { .hours = 0U, .minutes = 0U, .seconds = 0U };

static uint8_t timeWasChanged = 0U;

static ClockDate_t currentDate = { .day = 1U, .month = 1U, .year = 2026U };

static ClockDate_t editedDate = { .day = 1U, .month = 1U, .year = 2026U };

static uint8_t dateWasChanged = 0U;
static uint32_t lastRtcReadTick = 0U;
static uint8_t rtcReady = 0U;
static uint8_t currentBrightness = LEDS_BRIGHTNESS_MAX;
static uint8_t editedBrightness = LEDS_BRIGHTNESS_MAX;

static ClockMode_t currentMode = CLOCK_MODE_RUN;
static uint8_t displayWasOffBeforeMenu = 0U;

static void Clock_HandleMenuButton(void);
static void Clock_HandlePlusButton(void);
static void Clock_HandleMinusButton(void);
static void Clock_UpdateDisplay(void);
static uint8_t Clock_IsLeapYear(uint16_t year);
static uint8_t Clock_GetDaysInMonth(uint8_t month, uint16_t year);
static void Clock_ClampEditedDay(void);

void Clock_Init(void) {
	currentBrightness = Settings_LoadBrightness();
	editedBrightness = currentBrightness;
	LEDs_SetBrightness(currentBrightness);
	rtcReady = DS3231_IsReady();
	if (rtcReady != 0U) {
		if ((DS3231_ReadTime(&currentTime) != 0U)
				&& (DS3231_ReadDate(&currentDate) != 0U)) {
			LEDs_ShowTime(&currentTime);

			Clock_UpdateDisplay();
		} else {
			rtcReady = 0U;
			LEDs_AllOn();
		}
	} else {
		LEDs_AllOn();
	}
	lastRtcReadTick = HAL_GetTick();
}

void Clock_Update(void) {
	uint32_t now;
	ClockTime_t newTime;
	ClockDate_t newDate;
	uint8_t timeChanged;
	uint8_t dateChanged;

	if (currentMode != CLOCK_MODE_RUN) {
		return;
	}
	if (rtcReady == 0U) {
		return;
	}
	now = HAL_GetTick();
	if ((uint32_t) (now - lastRtcReadTick) < RTC_READ_INTERVAL_MS) {
		return;
	}
	lastRtcReadTick = now;
	if (DS3231_ReadTime(&newTime) == 0U) {
		return;
	}
	if (DS3231_ReadDate(&newDate) == 0U) {
		return;
	}
	timeChanged =
			((newTime.hours != currentTime.hours)
					|| (newTime.minutes != currentTime.minutes)
					|| (newTime.seconds != currentTime.seconds)) ? 1U : 0U;
	dateChanged =
			((newDate.day != currentDate.day)
					|| (newDate.month != currentDate.month)
					|| (newDate.year != currentDate.year)) ? 1U : 0U;
	if (timeChanged != 0U) {
		currentTime = newTime;
		LEDs_ShowTime(&currentTime);
		Buzzer_CheckTime(&currentTime);
	}
	if (dateChanged != 0U) {
		currentDate = newDate;
	}
	if ((timeChanged != 0U) || (dateChanged != 0U)) {
		Clock_UpdateDisplay();
	}
}

void Clock_HandleButton(ButtonEvent_t event) {
	switch (event) {
	case BUTTON_EVENT_MENU:
		Clock_HandleMenuButton();
		break;
	case BUTTON_EVENT_PLUS:
		Clock_HandlePlusButton();
		break;
	case BUTTON_EVENT_MINUS:
		Clock_HandleMinusButton();
		break;
	case BUTTON_EVENT_DISPLAY:
		Display_Toggle();
		if (Display_IsEnabled() != 0U) {
			Clock_UpdateDisplay();
		}
		break;
	case BUTTON_EVENT_BUZZER:
		Buzzer_Toggle();
		break;
	case BUTTON_EVENT_NONE:
	default:
		break;
	}
}

void Clock_RefreshDisplay(void) {
	Clock_UpdateDisplay();
}

static void Clock_HandleMenuButton(void) {
	switch (currentMode) {
	case CLOCK_MODE_RUN: {
		if (Display_IsEnabled() == 0U) {
			displayWasOffBeforeMenu = 1U;
			Display_Toggle();
		} else {
			displayWasOffBeforeMenu = 0U;
		}
		editedTime = currentTime;
		editedDate = currentDate;
		editedBrightness = currentBrightness;
		timeWasChanged = 0U;
		dateWasChanged = 0U;
		currentMode = CLOCK_MODE_SET_HOURS;
		LEDs_BlinkHours(1U);
		LEDs_BlinkMinutes(0U);
		LEDs_ShowTime(&editedTime);
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_HOURS: {
		currentMode = CLOCK_MODE_SET_MINUTES;
		LEDs_BlinkHours(0U);
		LEDs_BlinkMinutes(1U);
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_MINUTES: {
		currentMode = CLOCK_MODE_SET_DAY;
		LEDs_BlinkHours(0U);
		LEDs_BlinkMinutes(0U);
		LEDs_ShowTime(&editedTime);
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_DAY: {
		currentMode = CLOCK_MODE_SET_MONTH;
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_MONTH: {
		currentMode = CLOCK_MODE_SET_YEAR;
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_YEAR: {
		currentMode = CLOCK_MODE_SET_BRIGHTNESS;
		LEDs_SetBrightness(editedBrightness);
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_BRIGHTNESS: {
		uint8_t saveOk = 1U;
		currentBrightness = editedBrightness;
		LEDs_SetBrightness(currentBrightness);
		Settings_SaveBrightness(currentBrightness);
		if (timeWasChanged != 0U) {
			editedTime.seconds = 0U;
			if (DS3231_SetTime(&editedTime) == 0U) {
				saveOk = 0U;
			} else {
				currentTime = editedTime;
			}
		}
		if (dateWasChanged != 0U) {
			Clock_ClampEditedDay();
			if (DS3231_SetDate(&editedDate) == 0U) {
				saveOk = 0U;
			} else {
				currentDate = editedDate;
			}
		}
		currentMode = CLOCK_MODE_RUN;
		LEDs_BlinkHours(0U);
		LEDs_BlinkMinutes(0U);
		if (saveOk != 0U) {
			lastRtcReadTick = HAL_GetTick();
			LEDs_ShowTime(&currentTime);
			if (displayWasOffBeforeMenu != 0U) {
				if (Display_IsEnabled() != 0U) {
					Display_Toggle();
				}
				displayWasOffBeforeMenu = 0U;
			} else {
				Clock_UpdateDisplay();
			}
		} else {
			rtcReady = 0U;
			LEDs_AllOn();
		}
		break;
	}
	default: {
		currentMode = CLOCK_MODE_RUN;
		LEDs_BlinkHours(0U);
		LEDs_BlinkMinutes(0U);
		timeWasChanged = 0U;
		dateWasChanged = 0U;
		break;
	}
	}
}

static void Clock_HandlePlusButton(void) {
	switch (currentMode) {
	case CLOCK_MODE_SET_HOURS: {
		editedTime.hours++;
		if (editedTime.hours >= 24U) {
			editedTime.hours = 0U;
		}
		timeWasChanged = 1U;
		LEDs_ShowTime(&editedTime);
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_MINUTES: {
		editedTime.minutes++;
		if (editedTime.minutes >= 60U) {
			editedTime.minutes = 0U;
		}
		timeWasChanged = 1U;
		LEDs_ShowTime(&editedTime);
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_DAY: {
		uint8_t maximumDay;
		maximumDay = Clock_GetDaysInMonth(editedDate.month, editedDate.year);
		editedDate.day++;
		if (editedDate.day > maximumDay) {
			editedDate.day = 1U;
		}
		dateWasChanged = 1U;
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_MONTH: {
		editedDate.month++;
		if (editedDate.month > 12U) {
			editedDate.month = 1U;
		}
		Clock_ClampEditedDay();
		dateWasChanged = 1U;
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_YEAR: {
		editedDate.year++;
		if (editedDate.year > 2099U) {
			editedDate.year = 2000U;
		}
		Clock_ClampEditedDay();
		dateWasChanged = 1U;
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_BRIGHTNESS: {
		if (editedBrightness < LEDS_BRIGHTNESS_MAX) {
			editedBrightness++;
		}
		LEDs_SetBrightness(editedBrightness);
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_RUN:
	default:
		break;
	}
}

static void Clock_HandleMinusButton(void) {
	switch (currentMode) {
	case CLOCK_MODE_SET_HOURS: {
		if (editedTime.hours == 0U) {
			editedTime.hours = 23U;
		} else {
			editedTime.hours--;
		}
		timeWasChanged = 1U;
		LEDs_ShowTime(&editedTime);
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_MINUTES: {
		if (editedTime.minutes == 0U) {
			editedTime.minutes = 59U;
		} else {
			editedTime.minutes--;
		}
		timeWasChanged = 1U;
		LEDs_ShowTime(&editedTime);
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_DAY: {
		uint8_t maximumDay;
		maximumDay = Clock_GetDaysInMonth(editedDate.month, editedDate.year);
		if (editedDate.day <= 1U) {
			editedDate.day = maximumDay;
		} else {
			editedDate.day--;
		}
		dateWasChanged = 1U;
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_MONTH: {
		if (editedDate.month <= 1U) {
			editedDate.month = 12U;
		} else {
			editedDate.month--;
		}
		Clock_ClampEditedDay();
		dateWasChanged = 1U;
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_YEAR: {
		if (editedDate.year <= 2000U) {
			editedDate.year = 2099U;
		} else {
			editedDate.year--;
		}
		Clock_ClampEditedDay();
		dateWasChanged = 1U;
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_SET_BRIGHTNESS: {
		if (editedBrightness > LEDS_BRIGHTNESS_MIN) {
			editedBrightness--;
		}
		LEDs_SetBrightness(editedBrightness);
		Clock_UpdateDisplay();
		break;
	}
	case CLOCK_MODE_RUN:
	default:
		break;
	}
}

static uint8_t Clock_IsLeapYear(uint16_t year) {
	if ((year % 400U) == 0U) {
		return 1U;
	}
	if ((year % 100U) == 0U) {
		return 0U;
	}
	return ((year % 4U) == 0U) ? 1U : 0U;
}

static uint8_t Clock_GetDaysInMonth(uint8_t month, uint16_t year) {
	static const uint8_t days[] = { 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U,
			31U, 30U, 31U };
	if ((month < 1U) || (month > 12U)) {
		return 31U;
	}
	if ((month == 2U) && (Clock_IsLeapYear(year) != 0U)) {
		return 29U;
	}
	return days[month - 1U];
}

static void Clock_ClampEditedDay(void) {
	uint8_t maximumDay;
	maximumDay = Clock_GetDaysInMonth(editedDate.month, editedDate.year);
	if (editedDate.day > maximumDay) {
		editedDate.day = maximumDay;
	}
}

static void Clock_UpdateDisplay(void) {
	if (Display_IsEnabled() == 0U) {
		return;
	}
	switch (currentMode) {
	case CLOCK_MODE_RUN:
		Display_ShowDateTime(&currentTime, &currentDate, DHT11_GetData(),
				DHT11_IsReady(), LDR_IsLight());
		break;
	case CLOCK_MODE_SET_HOURS:
		Display_ShowSetting("HOURS", editedTime.hours, 2U);
		break;
	case CLOCK_MODE_SET_MINUTES:
		Display_ShowSetting("MINUTES", editedTime.minutes, 2U);
		break;
	case CLOCK_MODE_SET_DAY:
		Display_ShowSetting("DAY", editedDate.day, 2U);
		break;
	case CLOCK_MODE_SET_MONTH:
		Display_ShowSetting("MONTH", editedDate.month, 2U);
		break;
	case CLOCK_MODE_SET_YEAR:
		Display_ShowSetting("YEAR", editedDate.year, 4U);
		break;
	case CLOCK_MODE_SET_BRIGHTNESS:
		Display_ShowSetting("BRIGHT", editedBrightness, 1U);
		break;
	default:
		break;
	}
}
