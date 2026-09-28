

#ifndef INC_HC595_LEDS_H_
#define INC_HC595_LEDS_H_

#include <stdint.h>
#include "clock_time.h"

#define LEDS_BRIGHTNESS_MIN  1U
#define LEDS_BRIGHTNESS_MAX  4U

void LEDs_Init(void);

void LEDs_ShowTime(const ClockTime_t *time);

void LEDs_AllOn(void);

void LEDs_BlinkHours(uint8_t enable);

void LEDs_BlinkMinutes(uint8_t enable);

void LEDs_SetBrightness(uint8_t level);

void LEDs_TimerCallback(void);

void LEDs_SetDarkMode(uint8_t darkMode);


#endif /* INC_HC595_LEDS_H_ */
