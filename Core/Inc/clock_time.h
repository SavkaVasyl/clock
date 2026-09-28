

#ifndef INC_CLOCK_TIME_H_
#define INC_CLOCK_TIME_H_

#include <stdint.h>

typedef struct {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} ClockTime_t;

typedef struct {
	uint8_t day;
	uint8_t month;
	uint16_t year;
} ClockDate_t;

#endif /* INC_CLOCK_TIME_H_ */
