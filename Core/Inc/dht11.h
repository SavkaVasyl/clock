

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include <stdint.h>

typedef struct
{
    uint8_t temperature;
    uint8_t humidity;
} DHT11_Data_t;

void DHT11_Init(void);
void DHT11_Update(void);

const DHT11_Data_t *DHT11_GetData(void);
uint8_t DHT11_IsReady(void);

void DHT11_CaptureCallback(void);

#endif /* INC_DHT11_H_ */
