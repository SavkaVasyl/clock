#include "settings.h"

#include "stm32f4xx_hal.h"
#include "hc595_leds.h"

#define SETTINGS_FLASH_START      0x08060000U
#define SETTINGS_FLASH_END        0x08080000U
#define SETTINGS_EMPTY            0xFFFFFFFFU
#define SETTINGS_DEFAULT_BUZZER   1U

static uint8_t currentBrightness = LEDS_BRIGHTNESS_MAX;
static uint8_t currentBuzzer = SETTINGS_DEFAULT_BUZZER;
static uint32_t nextAddress = SETTINGS_FLASH_START;

void Settings_Init(void)
{
    uint32_t address;
    uint32_t value;
    uint8_t brightness;
    uint8_t buzzer;

    currentBrightness = LEDS_BRIGHTNESS_MAX;
    currentBuzzer = SETTINGS_DEFAULT_BUZZER;
    nextAddress = SETTINGS_FLASH_START;

    for (address = SETTINGS_FLASH_START;
         address < SETTINGS_FLASH_END;
         address += 4U)
    {
        value = *(volatile uint32_t *)address;
        if (value == SETTINGS_EMPTY)
        {
            nextAddress = address;
            return;
        }
        brightness = (uint8_t)(value & 0xFFU);
        buzzer = (uint8_t)((value >> 8U) & 0xFFU);
        if ((brightness >= LEDS_BRIGHTNESS_MIN) &&
            (brightness <= LEDS_BRIGHTNESS_MAX) &&
            (buzzer <= 1U))
        {
            currentBrightness = brightness;
            currentBuzzer = buzzer;
        }
    }
    nextAddress = SETTINGS_FLASH_END;
}

static void Settings_Save(void)
{
    uint32_t value;
    value =
        ((uint32_t)currentBuzzer << 8U) |
        (uint32_t)currentBrightness;

    HAL_FLASH_Unlock();
    if (nextAddress >= SETTINGS_FLASH_END)
    {
        FLASH_EraseInitTypeDef erase = {0};
        uint32_t sectorError = 0U;
        erase.TypeErase = FLASH_TYPEERASE_SECTORS;
        erase.Sector = FLASH_SECTOR_7;
        erase.NbSectors = 1U;
        erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        if (HAL_FLASHEx_Erase(
                &erase,
                &sectorError) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return;
        }
        nextAddress = SETTINGS_FLASH_START;
    }

    if (HAL_FLASH_Program(
            FLASH_TYPEPROGRAM_WORD,
            nextAddress,
            value) == HAL_OK)
    {
        nextAddress += 4U;
    }
    HAL_FLASH_Lock();
}

uint8_t Settings_LoadBrightness(void)
{
    return currentBrightness;
}

void Settings_SaveBrightness(uint8_t brightness)
{
    if ((brightness < LEDS_BRIGHTNESS_MIN) ||
        (brightness > LEDS_BRIGHTNESS_MAX))
    {
        return;
    }
    if (brightness == currentBrightness)
    {
        return;
    }
    currentBrightness = brightness;
    Settings_Save();
}

uint8_t Settings_LoadBuzzer(void)
{
    return currentBuzzer;
}

void Settings_SaveBuzzer(uint8_t enabled)
{
    if (enabled > 1U)
    {
        return;
    }
    if (enabled == currentBuzzer)
    {
        return;
    }
    currentBuzzer = enabled;
    Settings_Save();
}
