#include "mevent.h"
#include "custom/debug.h"

#define BIT_UP    0
#define BIT_DOWN  1
#define BIT_LEFT  2
#define BIT_RIGHT 3
#define BIT_SPIN  4
#define BIT_ON    5
#define BIT_RST   6
#define BIT_START 7

#define GPIO_UP    GPIOC
#define GPIO_DOWN  GPIOC
#define GPIO_LEFT  GPIOC
#define GPIO_RIGHT GPIOD
#define GPIO_SPIN  GPIOC
#define GPIO_ON    GPIOD
#define GPIO_RST   GPIOC
#define GPIO_START GPIOC

#define PORT_UP    GPIO_PIN_7
#define PORT_DOWN  GPIO_PIN_6
#define PORT_LEFT  GPIO_PIN_5
#define PORT_RIGHT GPIO_PIN_1
#define PORT_SPIN  GPIO_PIN_0
#define PORT_ON    GPIO_PIN_0
#define PORT_RST   GPIO_PIN_2
#define PORT_START GPIO_PIN_1

const GPIO_TypeDef * ButtonGPIO[8] = {GPIOC, GPIOC, GPIOC, GPIOD, GPIOC, GPIOD, GPIOC, GPIOC};
const u16 ButtonPort[8] = {GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_5, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_0, GPIO_PIN_2, GPIO_PIN_1};
const u8 ButtonEvent[8] = {
		EVENT_KEY_UP, EVENT_KEY_DOWN, EVENT_KEY_LEFT, EVENT_KEY_RIGHT,
		EVENT_KEY_SPIN, EVENT_KEY_ON, EVENT_KEY_RST, EVENT_KEY_START,
};
const u8 ButtonLongEvent[8] = {
		EVENT_KEY_UP_LONG, EVENT_KEY_DOWN_LONG, EVENT_KEY_LEFT_LONG, EVENT_KEY_RIGHT_LONG,
		EVENT_KEY_SPIN_LONG, EVENT_KEY_ON_LONG, EVENT_KEY_RST_LONG, EVENT_KEY_START_LONG,
};

#define LONG_PRESS_TIMEMS 1300
u8 buttonStatus[8] = {0};
u32 BtnPressedTime[8] = {0};

// ------------------------- BTN define -------------------------

#define EVT_COUNT 100

typedef struct {
	u8 event[EVT_COUNT + 1];
	u8 fast;
	u8 old;
	u8 count;
	u32 tick;
} MeventData;

MeventData me;

void Event_Init() {
	me.count = 0;
	me.fast = 0;
	me.old = 0;
	me.tick = 0;

	for (i8 i = 0; i < 8; i++) {
		buttonStatus[i] = 0;
		BtnPressedTime[i] = 0;
	}
}


i8 Event_Add(u8 event) {
	me.tick = HAL_GetTick();  // 只刷新时间
	if (event == 0) {
		return;
	}

	if (me.count >= EVT_COUNT) {
		print("event full.");
		return -1;
	}
	me.fast++;
	if (me.fast >= EVT_COUNT) {
		me.fast = 0;
	}
	me.event[me.fast] = event;
	me.count++;

	return 0;
}

u8 Event_Get() {
	if (me.count == 0) {
		return 0;
	}

	me.old++;
	if (me.old >= EVT_COUNT) {
		me.old = 0;
	}
	me.count--;
	return me.event[me.old];
}

u32 Event_GetLastEventDur() {
	u32 sysTick = HAL_GetTick();
	return (sysTick > me.tick) ? (sysTick - me.tick) : (0xFFFFFFFF - me.tick + sysTick);
}


void Event_CheckButton() {
	u32 dur = 0;
	u32 tick = HAL_GetTick();

	// BTN status
	// 0 release
	// 1 pressed down
	// 2 wait release
	for (i8 i = 0; i < 8; i++) {

		// 距离上一次按下的间隔
		u32 pressedTick = BtnPressedTime[i];
		if (tick < pressedTick) {
			dur = (0xFFFFFFFF - pressedTick) + tick;
		} else {
			dur = tick - pressedTick;
		}

		if ((ButtonGPIO[i]->IDR & ButtonPort[i]) == 0x00U) {
			// 按下
			Event_Add(0);
			if (buttonStatus[i] == 0) {
				// 之前是释放状态，但时间小于20ms，忽略抖动
				if (dur < 20) {
					continue;
				}
				// 记录按下时间
				buttonStatus[i] = 1;
				BtnPressedTime[i] = tick;
			} else if (buttonStatus[i] == 1) {
				// 之前是按下的状态
				if (dur < LONG_PRESS_TIMEMS) {
					// 时间过短，不予处理
					continue;
				} else {
					// duration 达到了长按的时长，触发长按事件，并将按键设定为等待抬起的状态
					Event_Add(ButtonLongEvent[i]);
					buttonStatus[i] = 2;
				}
			}
		} else {
			// 抬起
			if (buttonStatus[i] == 0) {
				// 之前是释放状态，现在又释放？消抖
				BtnPressedTime[i] = tick;
			} else if (buttonStatus[i] == 1) {
				// 之前记录了按下状态
				if (dur > 1000) {
					Event_Add(ButtonLongEvent[i]);
				} else {
					Event_Add(ButtonEvent[i]);
				}
				// 释放
				buttonStatus[i] = 0;
				BtnPressedTime[i] = tick;
			} else if (buttonStatus[i] == 2) {
				// 等待抬起的，现在释放
				buttonStatus[i] = 0;
				BtnPressedTime[i] = tick;
			}
		}
	}
}
