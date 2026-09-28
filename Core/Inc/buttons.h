

#ifndef INC_BUTTONS_H_
#define INC_BUTTONS_H_

#include <stdint.h>

typedef enum {
	BUTTON_EVENT_NONE = 0,
	BUTTON_EVENT_MENU,
	BUTTON_EVENT_PLUS,
	BUTTON_EVENT_MINUS,
	BUTTON_EVENT_DISPLAY,
	BUTTON_EVENT_BUZZER

} ButtonEvent_t;

void Buttons_Init(void);

ButtonEvent_t Buttons_GetEvent(void);

#endif /* INC_BUTTONS_H_ */
