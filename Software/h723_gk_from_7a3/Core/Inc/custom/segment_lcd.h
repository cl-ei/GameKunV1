#ifndef __SEGMENT_LCD_H
#define __SEGMENT_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "myt.h"

#define SEGLCD_BG_LED_GPIO_Port GPIOA
#define SEGLCD_BG_LED_PIN GPIO_PIN_9

#define SEGLCD_GRAM_LEN (20*2)

void SegLcd_Init(void);
void SegLcd_DrawOneFrame(void);
u64 SegLcd_GetTick();
void SegLcd_SetPrevBox(u8 data);
void SegLcd_SetTopValue(u16 data, u8 mode);
void SegLcd_SetLevelValue(u8 data, u8 mode);

#ifdef __cplusplus
}
#endif

#endif
