#include "display.h"
#include "custom/segment_lcd.h"
#include "custom/myt.h"
#include "custom/zf.h"

uint8_t progressBarMap[] = {0b10000, 0b100000, 0b10000000, 0b01000000};

extern uint16_t SegLcd_GRAM[];

void flushProgresBar() {
	static uint8_t mode = 0;
	static uint64_t tick = 0;

	u64 lcdTick = SegLcd_GetTick();
	if ((lcdTick - tick) > 120) {
		tick = lcdTick;
		mode = (mode + 1) % 4;
		SegLcd_SetPrevBox(progressBarMap[mode]);
	}
}

#define GRAM_LEN (20*200)
u16 GRAM[GRAM_LEN];
u16 bottomBorder = 0;
u8 cpOffset = 0;
i8 scrollDir = 0;

void renderOneChar(uint16_t c, uint8_t pos){
	uint32_t zStart = FontHeader[c]*20;
	uint16_t lineVar = 0;
	uint16_t lineOffset = pos*10;  // 7包含前面的数字

	for (uint8_t line = 0; line < 10; line++) {
		lineVar = (uint16_t)FontBody[zStart + 2*line];
		lineVar |= ((uint16_t)FontBody[zStart + (2*line+1)]) << 8;
		GRAM[line + lineOffset] = lineVar;
	}
}

void renderText(uint16_t* c, int16_t charSize) {
	cpOffset = 0;
	memset(GRAM, 0, GRAM_LEN*2);
	for (uint8_t i = 0; i < charSize; i++) {
		renderOneChar(*(c + i), i);
	}
	if (charSize <= 2) {
		bottomBorder = 0;
	}
	bottomBorder = 10*(charSize - 2);
}

int8_t scrollDown() {
	if (cpOffset >= bottomBorder) {
		return 1;
	} else {
		cpOffset ++;
		return 0;
	}
}

int8_t scrollUp() {
	if (cpOffset == 0) {
		return 1;
	} else {
		cpOffset --;
		return 0;
	}
}


void Scr_ShowAndScroll() {
	for (u8 i = 0; i < 20; i++) {
		SegLcd_GRAM[i] = GRAM[i+cpOffset];
	}
	if (scrollDir == 0) {
		scrollDir = scrollDown() == 0 ? 0 : 1;
	} else {
		scrollDir = scrollUp() == 0 ? 1 : 0;
	}
}

void Scr_Clean() {
	memset(SegLcd_GRAM, 0, SEGLCD_GRAM_LEN*2);
}

const u8 NumModel[10][5] = {
		{0b111, 0b101, 0b101, 0b101, 0b111},
		{0b001, 0b001, 0b001, 0b001, 0b001},
		{0b111, 0b001, 0b111, 0b100, 0b111},
		{0b111, 0b001, 0b111, 0b001, 0b111},
		{0b101, 0b101, 0b111, 0b001, 0b001},
		{0b111, 0b100, 0b111, 0b001, 0b111},
		{0b111, 0b100, 0b111, 0b101, 0b111},
		{0b111, 0b001, 0b001, 0b001, 0b001},
		{0b111, 0b101, 0b111, 0b101, 0b111},
		{0b111, 0b101, 0b111, 0b001, 0b111}
};

void copyNumToGram(u8 x, u8 y, u8 num) {
	for (i8 j = 0; j < 5; j++) {
		for (i8 i = 0; i < 3; i++) {
			if ((NumModel[num][j] & (1 << i)) != 0) {
				SegLcd_GRAM[j + y] |= 1 << (x + (2 - i));
			} else {
				SegLcd_GRAM[j + y] &= ~(1 << (x + (2 - i)));
			}
		}
	}
}

extern u8 SegLcd_Body;
extern u8 SegLcd_PrevBox;
void Scr_RenderTime(u8 hour, u8 min, u8 second) {
	const i8 offset =8;
	const i8 x = 2;
	static u64 tick = 0;
	static u8 lastH = 255;
	static u8 lastM = 255;
	static u8 lastS = 255;
	static u8 heart = 0;
	static u64 handTick = 0;
	static u8 handState = 0;

	u64 sysTick = SegLcd_GetTick();
	if (sysTick - tick > 800) {
		tick = sysTick;

		// render heart
		if ((heart++) & 1) {
			SegLcd_GRAM[0 + offset] |= 0b110011 << x;
			SegLcd_GRAM[1 + offset] |= 0b111111 << x;
			SegLcd_GRAM[2 + offset] |= 0b111111 << x;
			SegLcd_GRAM[3 + offset] |= 0b011110 << x;
			SegLcd_GRAM[4 + offset] |= 0b001100 << x;
		} else {
			SegLcd_GRAM[0 + offset] &= ~(0b110011 << x);
			SegLcd_GRAM[1 + offset] &= ~(0b111111 << x);
			SegLcd_GRAM[2 + offset] &= ~(0b111111 << x);
			SegLcd_GRAM[3 + offset] &= ~(0b011110 << x);
			SegLcd_GRAM[4 + offset] &= ~(0b001100 << x);
		}
	}

	// render hour
	if (hour != lastH) {
		lastH = hour;
		copyNumToGram(1, 1, (u8)(hour / 10));
		copyNumToGram(5, 2, hour % 10);
	}
	// render min
	if (min != lastM) {
		lastM = min;
		copyNumToGram(1, 13, (u8)(min / 10));
		copyNumToGram(5, 14, min % 10);
	}
	if (second != lastS) {
		lastS = second;
		SegLcd_SetTopValue(second, 1);
	}

	// render body
	if (sysTick - handTick > 100) {
		handTick = sysTick;
		handState++;
		if (handState >= 123) {
			handState = 0;
		}
		if (handState == 0 || handState == 2) {
			SegLcd_Body = 0b011;
		}else if (handState == 1 || handState == 3) {
			SegLcd_Body = 0b101;
		}else if (handState == 4) {
			SegLcd_PrevBox = 0b1;
		}else if (handState == 5) {
			SegLcd_PrevBox = 0b10;
		}else if (handState == 6) {
			SegLcd_PrevBox = 0b100;
		}else if (handState == 7) {
			SegLcd_PrevBox = 0b1000;
		}else if (handState == 8) {
			SegLcd_PrevBox = 0;
		}
	}
}
u32 tuning = 0;

void Scr_FlashHourMin(u32 tick, u8 hour, u8 min, u8 second, u8 flash) {
	// flash 0: hour; 1: min
	static u32 last = 0;
	static u8 mode = 0;

	if (tick - last >= 400) {
		last = tick;

		Scr_Clean();
		mode++;

		if ((flash != 0) || (mode & 1U)) {
			copyNumToGram(1, 1, (u8)(hour / 10));
			copyNumToGram(5, 2, hour % 10);
		}

		if ((flash != 1) || (mode & 1U)) {
			copyNumToGram(1, 13, (u8)(min / 10));
			copyNumToGram(5, 14, min % 10);
		}
		SegLcd_SetTopValue(second, 1);
	}
}


void Scr_RenderUSBPage(void) {
	Scr_Clean();
	SegLcd_GRAM[1] = 0x20;
	SegLcd_GRAM[2] = 0x70;
	SegLcd_GRAM[3] = 0x20;
	SegLcd_GRAM[4] = 0x20;

	SegLcd_GRAM[5] = 0xA4;
	SegLcd_GRAM[6] = 0xA4;
	SegLcd_GRAM[7] = 0xA4;
	SegLcd_GRAM[8] = 0xA4;

	SegLcd_GRAM[9] = 0x64;
	SegLcd_GRAM[10] = 0x28;
	SegLcd_GRAM[11] = 0x30;
	SegLcd_GRAM[12] = 0x20;
	SegLcd_GRAM[13] = 0x20;

	SegLcd_GRAM[14] = 0x70;
	SegLcd_GRAM[15] = 0x70;

	SegLcd_SetTopValue(0, 1);
	SegLcd_Body = 0;
}
