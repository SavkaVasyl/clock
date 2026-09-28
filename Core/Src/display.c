
#include "display.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

static uint8_t displayInitialized = 0U;
static uint8_t displayEnabled = 1U;

void Display_Init(void) {
	ssd1306_Init();
	ssd1306_Fill(Black);
	ssd1306_SetCursor(31U, 5U);
	ssd1306_WriteString("CLOCK", Font_11x18, White);
	ssd1306_UpdateScreen();

	displayInitialized = 1U;
	displayEnabled = 1U;
}

void Display_ShowDateTime(const ClockTime_t *time, const ClockDate_t *date,
		const DHT11_Data_t *dht, uint8_t dhtReady, uint8_t isLight) {
	char timeText[9];
	char dateText[11];
	char dhtText[9];
	const char *lightText;

	if ((displayInitialized == 0U) || (displayEnabled == 0U)) {
		return;
	}

	if ((time == NULL) || (date == NULL)) {
		return;
	}
	if ((time->hours > 23U) || (time->minutes > 59U) || (time->seconds > 59U)
			|| (date->day < 1U) || (date->day > 31U) || (date->month < 1U)
			|| (date->month > 12U) || (date->year < 2000U)
			|| (date->year > 2099U)) {
		return;
	}
	timeText[0] = (char) ('0' + time->hours / 10U);
	timeText[1] = (char) ('0' + time->hours % 10U);
	timeText[2] = ':';

	timeText[3] = (char) ('0' + time->minutes / 10U);
	timeText[4] = (char) ('0' + time->minutes % 10U);
	timeText[5] = ':';

	timeText[6] = (char) ('0' + time->seconds / 10U);
	timeText[7] = (char) ('0' + time->seconds % 10U);
	timeText[8] = '\0';

	dateText[0] = (char) ('0' + date->day / 10U);
	dateText[1] = (char) ('0' + date->day % 10U);
	dateText[2] = '.';

	dateText[3] = (char) ('0' + date->month / 10U);
	dateText[4] = (char) ('0' + date->month % 10U);
	dateText[5] = '.';

	dateText[6] = (char) ('0' + (date->year / 1000U) % 10U);

	dateText[7] = (char) ('0' + (date->year / 100U) % 10U);

	dateText[8] = (char) ('0' + (date->year / 10U) % 10U);

	dateText[9] = (char) ('0' + date->year % 10U);

	dateText[10] = '\0';

	if ((dhtReady != 0U) && (dht != NULL)) {
		dhtText[0] = (char) ('0' + dht->temperature / 10U);
		dhtText[1] = (char) ('0' + dht->temperature % 10U);
		dhtText[2] = 'C';
		dhtText[3] = ' ';
		dhtText[4] = ' ';
		dhtText[5] = (char) ('0' + dht->humidity / 10U);
		dhtText[6] = (char) ('0' + dht->humidity % 10U);
		dhtText[7] = '%';
		dhtText[8] = '\0';
	} else {
		dhtText[0] = '-';
		dhtText[1] = '-';
		dhtText[2] = 'C';
		dhtText[3] = ' ';
		dhtText[4] = ' ';
		dhtText[5] = '-';
		dhtText[6] = '-';
		dhtText[7] = '%';
		dhtText[8] = '\0';
	}

	lightText = (isLight != 0U) ? "LIGHT" : "DARK";

	ssd1306_Fill(Black);
	ssd1306_SetCursor(20U, 0U);
	ssd1306_WriteString(timeText, Font_11x18, White);
	ssd1306_SetCursor(29U, 21U);
	ssd1306_WriteString(dateText, Font_7x10, White);
	ssd1306_SetCursor(35U, 36U);
	ssd1306_WriteString(dhtText, Font_7x10, White);
	ssd1306_SetCursor(35U, 51U);
	ssd1306_WriteString((char*) lightText, Font_7x10, White);
	ssd1306_UpdateScreen();
}

static void Display_FormatNumber(uint16_t value, uint8_t digits, char *text) {
	for (uint8_t i = 0U; i < digits; i++) {
		uint8_t position;
		position = (uint8_t) (digits - 1U - i);
		text[position] = (char) ('0' + (value % 10U));
		value /= 10U;
	}
	text[digits] = '\0';
}

void Display_ShowSetting(const char *title, uint16_t value, uint8_t digits) {
	char valueText[6];
	uint8_t x;
	if ((displayInitialized == 0U) || (displayEnabled == 0U)) {
		return;
	}
	if (title == NULL) {
		return;
	}
	if ((digits == 0U) || (digits > 5U)) {
		return;
	}
	Display_FormatNumber(value, digits, valueText);
	ssd1306_Fill(Black);
	ssd1306_SetCursor(20U, 2U);
	ssd1306_WriteString((char*) title, Font_11x18, White);
	if (digits <= 2U) {
		x = 48U;
	} else {
		x = 32U;
	}
	ssd1306_SetCursor(x, 34U);
	ssd1306_WriteString(valueText, Font_16x26, White);
	ssd1306_UpdateScreen();
}

void Display_Toggle(void) {
	if (displayInitialized == 0U) {
		return;
	}
	displayEnabled ^= 1U;
	ssd1306_SetDisplayOn(displayEnabled);
}

uint8_t Display_IsEnabled(void) {
	if (displayInitialized == 0U) {
		return 0U;
	}
	return displayEnabled;
}
