

#ifndef INC_LDR_H_
#define INC_LDR_H_

#include <stdint.h>

void LDR_Init(void);

uint8_t LDR_Update(void);

uint8_t LDR_IsLight(void);

#endif /* INC_LDR_H_ */
