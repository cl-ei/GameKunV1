#ifndef __MEVENT_H
#define __MEVENT_H

#include "stm32h7xx_hal.h"
#include "custom/myt.h"

#define EVENT_KEY_UP    1
#define EVENT_KEY_DOWN  2
#define EVENT_KEY_LEFT  3
#define EVENT_KEY_RIGHT 4
#define EVENT_KEY_ON    5
#define EVENT_KEY_RST   6
#define EVENT_KEY_START 7
#define EVENT_KEY_SPIN  8

#define EVENT_KEY_UP_LONG    9
#define EVENT_KEY_DOWN_LONG  10
#define EVENT_KEY_LEFT_LONG  11
#define EVENT_KEY_RIGHT_LONG 12
#define EVENT_KEY_ON_LONG    13
#define EVENT_KEY_RST_LONG   14
#define EVENT_KEY_START_LONG 15
#define EVENT_KEY_SPIN_LONG  16

#define EVENT_MUSIC_END 17



void Event_Init();
i8 Event_Add(u8 event);
u8 Event_Get();
u32 Event_GetLastEventDur();
void Event_CheckButton();

#endif
