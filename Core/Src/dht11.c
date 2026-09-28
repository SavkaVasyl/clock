

#include "dht11.h"
#include "main.h"
#include <stddef.h>

extern TIM_HandleTypeDef htim11;

#define DHT11_GPIO_PORT          GPIOB
#define DHT11_GPIO_PIN           GPIO_PIN_9

#define DHT11_BITS_COUNT         40U
#define DHT11_BYTES_COUNT        5U
#define DHT11_TIMEOUT_MS         10U
#define DHT11_ONE_THRESHOLD      50U
#define DHT11_READ_INTERVAL_MS   1000U

static volatile uint8_t bytes[DHT11_BYTES_COUNT];
static volatile uint8_t bitIndex;
static volatile uint8_t responseCaptured;
static volatile uint8_t captureDone;
static volatile uint8_t captureError;
static volatile uint16_t riseTime;

static DHT11_Data_t dht11Data;
static uint8_t dht11Ready;
static uint32_t lastReadTick;

static void DHT11_SetOutput(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = DHT11_GPIO_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &gpio);
}

static void DHT11_SetTimerInput(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = DHT11_GPIO_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF3_TIM11;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &gpio);
}

static void DHT11_SetIdle(void)
{
    DHT11_SetOutput();
    HAL_GPIO_WritePin(
        DHT11_GPIO_PORT,
        DHT11_GPIO_PIN,
        GPIO_PIN_SET
    );
}


static uint8_t DHT11_Read(DHT11_Data_t *data)
{
    uint32_t startTick;
    if (data == NULL)
    {
        return 0U;
    }
    for (uint8_t i = 0U; i < DHT11_BYTES_COUNT; i++)
    {
        bytes[i] = 0U;
    }
    bitIndex = 0U;
    responseCaptured = 0U;
    captureDone = 0U;
    captureError = 0U;
    riseTime = 0U;
    HAL_TIM_IC_Stop_IT(
        &htim11,
        TIM_CHANNEL_1
    );
    DHT11_SetOutput();
    HAL_GPIO_WritePin(
        DHT11_GPIO_PORT,
        DHT11_GPIO_PIN,
        GPIO_PIN_RESET
    );
    HAL_Delay(18U);
    __HAL_TIM_SET_COUNTER(&htim11, 0U);
    __HAL_TIM_SET_CAPTUREPOLARITY(
        &htim11,
        TIM_CHANNEL_1,
        TIM_INPUTCHANNELPOLARITY_RISING
    );
    __HAL_TIM_CLEAR_FLAG(
        &htim11,
        TIM_FLAG_CC1
    );
    HAL_NVIC_ClearPendingIRQ(
        TIM1_TRG_COM_TIM11_IRQn
    );
    HAL_GPIO_WritePin(
        DHT11_GPIO_PORT,
        DHT11_GPIO_PIN,
        GPIO_PIN_SET
    );
    DHT11_SetTimerInput();
    if (HAL_TIM_IC_Start_IT(
            &htim11,
            TIM_CHANNEL_1) != HAL_OK)
    {
        DHT11_SetIdle();
        return 0U;
    }
    startTick = HAL_GetTick();
    while ((captureDone == 0U) &&
           (captureError == 0U))
    {
        if ((uint32_t)(HAL_GetTick() - startTick) >=
            DHT11_TIMEOUT_MS)
        {
            captureError = 1U;
        }
    }
    HAL_TIM_IC_Stop_IT(
        &htim11,
        TIM_CHANNEL_1
    );
    __HAL_TIM_CLEAR_FLAG(
        &htim11,
        TIM_FLAG_CC1
    );
    DHT11_SetIdle();
    if (captureError != 0U)
    {
        return 0U;
    }
    if ((uint8_t)(
            bytes[0] +
            bytes[1] +
            bytes[2] +
            bytes[3]) != bytes[4])
    {
        return 0U;
    }
    data->humidity = bytes[0];
    data->temperature = bytes[2];
    return 1U;
}


void DHT11_Init(void)
{
    DHT11_SetIdle();
    dht11Data.temperature = 0U;
    dht11Data.humidity = 0U;
    dht11Ready = 0U;
    lastReadTick = HAL_GetTick() - DHT11_READ_INTERVAL_MS;
}


void DHT11_Update(void)
{
    uint32_t now;
    DHT11_Data_t newData;
    now = HAL_GetTick();
    if ((uint32_t)(now - lastReadTick) <
        DHT11_READ_INTERVAL_MS)
    {
        return;
    }
    lastReadTick = now;
    if (DHT11_Read(&newData) != 0U)
    {
        dht11Data = newData;
        dht11Ready = 1U;
    }
}

const DHT11_Data_t *DHT11_GetData(void)
{
    return &dht11Data;
}

uint8_t DHT11_IsReady(void)
{
    return dht11Ready;
}

void DHT11_CaptureCallback(void)
{
    uint16_t capture;
    uint16_t pulseWidth;
    capture = (uint16_t)HAL_TIM_ReadCapturedValue(
        &htim11,
        TIM_CHANNEL_1
    );
    if ((TIM11->CCER & TIM_CCER_CC1P) == 0U)
    {
        riseTime = capture;
        __HAL_TIM_SET_CAPTUREPOLARITY(
            &htim11,
            TIM_CHANNEL_1,
            TIM_INPUTCHANNELPOLARITY_FALLING
        );
        return;
    }
    if (capture >= riseTime)
    {
        pulseWidth = capture - riseTime;
    }
    else
    {
        pulseWidth =
            (uint16_t)(65535U - riseTime + capture + 1U);
    }
    __HAL_TIM_SET_CAPTUREPOLARITY(
        &htim11,
        TIM_CHANNEL_1,
        TIM_INPUTCHANNELPOLARITY_RISING
    );
    if (responseCaptured == 0U)
    {
        responseCaptured = 1U;
        return;
    }
    if (bitIndex >= DHT11_BITS_COUNT)
    {
        captureError = 1U;
        return;
    }
    if (pulseWidth > DHT11_ONE_THRESHOLD)
    {
        bytes[bitIndex / 8U] |=
            (uint8_t)(1U << (7U - (bitIndex % 8U)));
    }
    bitIndex++;
    if (bitIndex >= DHT11_BITS_COUNT)
    {
        captureDone = 1U;
    }
}
