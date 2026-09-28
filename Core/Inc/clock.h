

#ifndef INC_CLOCK_H_
#define INC_CLOCK_H_

#include "buttons.h"

void Clock_Init(void);
void Clock_Update(void);
void Clock_HandleButton(ButtonEvent_t event);
void Clock_RefreshDisplay(void);

#endif /* INC_CLOCK_H_ */
