

#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#include <stdint.h>

void Settings_Init(void);

uint8_t Settings_LoadBrightness(void);
void Settings_SaveBrightness(uint8_t brightness);

uint8_t Settings_LoadBuzzer(void);
void Settings_SaveBuzzer(uint8_t enabled);

#endif /* INC_SETTINGS_H_ */
