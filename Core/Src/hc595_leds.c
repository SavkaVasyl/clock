

#include "hc595_leds.h"
#include "main.h"

#define LEDS_BLINK_PERIOD_TICKS   250U
#define LEDS_PWM_PHASE_COUNT       8U

static volatile uint16_t blinkCounter = 0U;
static volatile uint8_t blinkState = 1U;

static volatile uint8_t blinkHours = 0U;
static volatile uint8_t blinkMinutes = 0U;

static volatile ClockTime_t currentDisplayTime;

static volatile uint8_t brightnessLevel = LEDS_BRIGHTNESS_MAX;
static volatile uint8_t pwmPhase = 0U;

static volatile uint8_t allOnMode = 0U;
static volatile uint8_t darkModeEnabled = 0U;

static void HC595_PulseClock(void) {
	HAL_GPIO_WritePin(
	HC595_CLOCK_GPIO_Port,
	HC595_CLOCK_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(
	HC595_CLOCK_GPIO_Port,
	HC595_CLOCK_Pin, GPIO_PIN_RESET);
}

static void HC595_PulseLatch(void) {
	HAL_GPIO_WritePin(
	HC595_LATCH_GPIO_Port,
	HC595_LATCH_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(
	HC595_LATCH_GPIO_Port,
	HC595_LATCH_Pin, GPIO_PIN_RESET);
}

static void HC595_SendByte(uint8_t value) {
	for (int8_t bit = 7; bit >= 0; bit--) {
		GPIO_PinState pinState;
		if ((value & (1U << bit)) != 0U) {
			pinState = GPIO_PIN_SET;
		} else {
			pinState = GPIO_PIN_RESET;
		}
		HAL_GPIO_WritePin(
		HC595_DATA_GPIO_Port,
		HC595_DATA_Pin, pinState);
		HC595_PulseClock();
	}
}

static void HC595_Send24(uint8_t register3, uint8_t register2, uint8_t register1) {
	HAL_GPIO_WritePin(
	HC595_LATCH_GPIO_Port,
	HC595_LATCH_Pin, GPIO_PIN_RESET);
	HC595_SendByte(register3);
	HC595_SendByte(register2);
	HC595_SendByte(register1);
	HC595_PulseLatch();
}

void LEDs_Init(void)
{
    HAL_GPIO_WritePin(
        HC595_CLOCK_GPIO_Port,
        HC595_CLOCK_Pin,
        GPIO_PIN_RESET
    );
    HAL_GPIO_WritePin(
        HC595_DATA_GPIO_Port,
        HC595_DATA_Pin,
        GPIO_PIN_RESET
    );
    HAL_GPIO_WritePin(
        HC595_LATCH_GPIO_Port,
        HC595_LATCH_Pin,
        GPIO_PIN_RESET
    );
    currentDisplayTime.hours = 0U;
    currentDisplayTime.minutes = 0U;
    currentDisplayTime.seconds = 0U;

    blinkCounter = 0U;
    blinkState = 1U;
    blinkHours = 0U;
    blinkMinutes = 0U;

    brightnessLevel = LEDS_BRIGHTNESS_MAX;
    pwmPhase = 0U;

    allOnMode = 0U;
    darkModeEnabled = 0U;

    HC595_Send24(0x00U, 0x00U, 0x00U);
}

void LEDs_ShowTime(const ClockTime_t *time)
{
    if (time == NULL)
    {
        return;
    }
    if ((time->hours > 23U) ||
        (time->minutes > 59U) ||
        (time->seconds > 59U))
    {
        return;
    }
    allOnMode = 0U;
    currentDisplayTime = *time;
}

void LEDs_AllOn(void)
{
    allOnMode = 1U;
}

void LEDs_BlinkHours(uint8_t enable)
{
    blinkHours = (enable != 0U) ? 1U : 0U;
    blinkCounter = 0U;
    blinkState = 1U;
}

void LEDs_BlinkMinutes(uint8_t enable)
{
    blinkMinutes = (enable != 0U) ? 1U : 0U;
    blinkCounter = 0U;
    blinkState = 1U;
}

void LEDs_TimerCallback(void)
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t effectiveBrightness;
    if (allOnMode != 0U)
    {
        hours = 0xFFU;
        minutes = 0xFFU;
        seconds = 0xFFU;
    }
    else
    {
        hours = currentDisplayTime.hours;
        minutes = currentDisplayTime.minutes;
        seconds = currentDisplayTime.seconds;
        if ((blinkHours != 0U) || (blinkMinutes != 0U))
        {
            blinkCounter++;
            if (blinkCounter >= LEDS_BLINK_PERIOD_TICKS)
            {
                blinkCounter = 0U;
                blinkState ^= 1U;
            }
        }
        else
        {
            blinkCounter = 0U;
            blinkState = 1U;
        }
        if (blinkState == 0U)
        {
            if (blinkHours != 0U)
            {
                hours = 0U;
            }
            if (blinkMinutes != 0U)
            {
                minutes = 0U;
            }
        }
    }
    if (darkModeEnabled != 0U)
    {
        effectiveBrightness = brightnessLevel;
    }
    else
    {
        effectiveBrightness = brightnessLevel * 2U;
    }
    if (pwmPhase < effectiveBrightness)
    {
        HC595_Send24(
            seconds,
            minutes,
            hours
        );
    }
    else
    {
        HC595_Send24(
            0x00U,
            0x00U,
            0x00U
        );
    }
    pwmPhase++;
    if (pwmPhase >= LEDS_PWM_PHASE_COUNT)
    {
        pwmPhase = 0U;
    }
}

void LEDs_SetBrightness(uint8_t level)
{
    if (level < LEDS_BRIGHTNESS_MIN)
    {
        level = LEDS_BRIGHTNESS_MIN;
    }
    if (level > LEDS_BRIGHTNESS_MAX)
    {
        level = LEDS_BRIGHTNESS_MAX;
    }
    brightnessLevel = level;
    pwmPhase = 0U;
}

void LEDs_SetDarkMode(uint8_t darkMode)
{
    darkModeEnabled =
        (darkMode != 0U) ? 1U : 0U;
    pwmPhase = 0U;
}
