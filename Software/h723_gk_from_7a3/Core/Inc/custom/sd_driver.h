#ifndef __SD_DRIVER_H
#define __SD_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"
int8_t CL_SD_Init(void);
int8_t CL_SD_DeInit(void);
HAL_StatusTypeDef CL_getSDState();
HAL_StatusTypeDef CL_readSdInfo();
HAL_StatusTypeDef CL_sdReadBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
HAL_StatusTypeDef CL_sdWriteBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);

#ifdef __cplusplus
}
#endif

#endif
