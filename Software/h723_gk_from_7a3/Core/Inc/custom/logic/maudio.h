#ifndef __MAUDIO_H
#define __MAUDIO_H

#include "stm32h7xx_hal.h"
#include "custom/myt.h"


i8 Audio_Init();
i8 Audio_GetSongCount();
int8_t Audio_Start(i8 index);
void Audio_Pause();
void Audio_Play();
void Audio_LoadBuff();
u8 Audio_IsPlaying();
u32 Audio_GetSongTime();
void Audio_RenderUI(u32 tick);
#endif
