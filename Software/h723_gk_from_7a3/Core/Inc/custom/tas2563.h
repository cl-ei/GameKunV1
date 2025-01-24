#ifndef __TAS2563_H
#define __TAS2563_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

int8_t TAS2563_Init(uint8_t *buff, uint32_t len);
void TAS2563_Stop();
int8_t TAS2563_GetAndClearFlag();

#ifdef __cplusplus
}
#endif

#endif
