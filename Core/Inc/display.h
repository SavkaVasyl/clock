

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include "dht11.h"
#include "clock_time.h"


void Display_Init(void);

void Display_ShowDateTime(const ClockTime_t *time, const ClockDate_t *date,  const DHT11_Data_t *dht,
	    uint8_t dhtReady, uint8_t isLight);

void Display_ShowSetting(const char *title,
                         uint16_t value,
                         uint8_t digits);

void Display_Toggle(void);

uint8_t Display_IsEnabled(void);

#endif /* INC_DISPLAY_H_ */
