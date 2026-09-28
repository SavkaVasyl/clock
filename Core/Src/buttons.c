

#include "buttons.h"
#include "main.h"

#define BUTTON_DEBOUNCE_MS  40U

#define BUTTON_COUNT \
    ((uint8_t)(sizeof(buttons) / sizeof(buttons[0])))

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;

	GPIO_PinState rawState;
	GPIO_PinState stableState;

	uint32_t lastChangeTick;
	ButtonEvent_t event;
} Button_t;

static Button_t buttons[] = { { .port = BUTTON_MENU_GPIO_Port, .pin =
		BUTTON_MENU_Pin, .rawState = GPIO_PIN_SET, .stableState = GPIO_PIN_SET,
		.lastChangeTick = 0U, .event = BUTTON_EVENT_MENU },

{ .port = BUTTON_PLUS_GPIO_Port, .pin = BUTTON_PLUS_Pin, .rawState =
		GPIO_PIN_SET, .stableState = GPIO_PIN_SET, .lastChangeTick = 0U,
		.event = BUTTON_EVENT_PLUS },

{ .port = BUTTON_MINUS_GPIO_Port, .pin = BUTTON_MINUS_Pin, .rawState =
		GPIO_PIN_SET, .stableState = GPIO_PIN_SET, .lastChangeTick = 0U,
		.event = BUTTON_EVENT_MINUS }, { .port = BUTTON_DISPLAY_GPIO_Port,
		.pin =
		BUTTON_DISPLAY_Pin, .rawState = GPIO_PIN_SET, .stableState =
				GPIO_PIN_SET, .lastChangeTick = 0U, .event =
				BUTTON_EVENT_DISPLAY },
		{ .port = BUTTON_BUZZER_GPIO_Port, .pin = BUTTON_BUZZER_Pin, .rawState =
				GPIO_PIN_SET, .stableState = GPIO_PIN_SET, .lastChangeTick = 0U,
				.event = BUTTON_EVENT_BUZZER } };

void Buttons_Init(void) {
	uint32_t now = HAL_GetTick();

	for (uint8_t i = 0U; i < BUTTON_COUNT; i++) {
		GPIO_PinState currentState;

		currentState = HAL_GPIO_ReadPin(buttons[i].port, buttons[i].pin);

		buttons[i].rawState = currentState;
		buttons[i].stableState = currentState;
		buttons[i].lastChangeTick = now;
	}
}

ButtonEvent_t Buttons_GetEvent(void) {
	uint32_t now = HAL_GetTick();

	for (uint8_t i = 0U; i < BUTTON_COUNT; i++) {
		GPIO_PinState currentState;

		currentState = HAL_GPIO_ReadPin(buttons[i].port, buttons[i].pin);

		if (currentState != buttons[i].rawState) {
			buttons[i].rawState = currentState;
			buttons[i].lastChangeTick = now;
		}

		if ((uint32_t) (now - buttons[i].lastChangeTick) < BUTTON_DEBOUNCE_MS) {
			continue;
		}

		if (buttons[i].stableState != buttons[i].rawState) {
			buttons[i].stableState = buttons[i].rawState;
			if (buttons[i].stableState == GPIO_PIN_RESET) {
				return buttons[i].event;
			}
		}
	}

	return BUTTON_EVENT_NONE;
}
