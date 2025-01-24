#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "stm32h7xx_hal.h"
#include "custom/myt.h"


void flushProgresBar();
void Scr_Clean(void);
void Scr_RenderTime(u8 hour, u8 min, u8 second);
void Scr_FlashHourMin(u32 tick, u8 hour, u8 min, u8 second, u8 flash);
void Scr_RenderUSBPage(void);
#endif
