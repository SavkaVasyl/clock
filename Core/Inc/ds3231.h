

#ifndef INC_DS3231_H_
#define INC_DS3231_H_

#include <stdint.h>
#include "clock_time.h"

uint8_t DS3231_IsReady(void);

uint8_t DS3231_ReadTime(ClockTime_t *time);
uint8_t DS3231_SetTime(const ClockTime_t *time);

uint8_t DS3231_ReadDate(ClockDate_t *date);
uint8_t DS3231_SetDate(const ClockDate_t *date);

#endif /* INC_DS3231_H_ */
