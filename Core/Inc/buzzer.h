

#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include "clock_time.h"

void Buzzer_Init(void);
void Buzzer_Toggle(void);
void Buzzer_CheckTime(const ClockTime_t *time);
void Buzzer_TimerCallback(void);

#endif /* INC_BUZZER_H_ */
