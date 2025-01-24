#include "segment_lcd.h"
#include "stm32h7xx_hal.h"

// 每个元素代表一行，低位在左
uint16_t SegLcd_GRAM[SEGLCD_GRAM_LEN];

uint16_t SegLcd_GRAMOffset = 0;
uint16_t SegLcd_TopCnt = 0;
// 0: 全部显示,包括0;
// 1: 只显示两位
// 2: 只显示第1、3、4位（第二位为空）
uint8_t SegLcd_TopCntMode = 0;

uint8_t SegLcd_PrevBox = 0;

uint8_t SegLcd_Speed = 0;
uint8_t SegLcd_SpeedMode = 1;
uint8_t SegLcd_Level = 0;
uint8_t SegLcd_LevelMode = 1;
uint64_t SegLcd_TickMs = 0;

u8 SegLcd_ShowMusic = 0;
u8 SegLcd_Body = 0x07;  // 0b111

static uint8_t topNumMap[10][2] = {
		// com first, com 2nd
		{0b101000, 0b111100, }, // 数字0
		{0b101000, 0,        }, // 数字1
		{0b110000, 0b101100, }, // 数字2
		{0b111000, 0b100100, }, // 数字3
		{0b111000, 0b010000, }, // 数字4
		{0b011000, 0b110100, }, // 数字5
		{0b011000, 0b111100, }, // 数字6
		{0b101000, 0b100000, }, // 数字7
		{0b111000, 0b111100, }, // 数字8
		{0b111000, 0b110100, }, // 数字9
		{0b111000, 0b110100, }, // 10 冒号
};
static uint8_t PrevBoxMap[2][4] = {
		{0b100000, 0b010000, 0b001000, 0b000100},
		{0b010000, 0b001000, 0b100000, 0b000100},
};
static uint8_t SpeedMap[10][7] = {  // com 2 ~ 8, total: 7
	//   d  c  e  g  b  f  a
 		{1, 1, 1, 0, 1, 1, 1}, // 数字0
 		{0, 1, 0, 0, 1, 0, 0}, // 数字1
 		{1, 0, 1, 1, 1, 0, 1}, // 数字2
 		{1, 1, 0, 1, 1, 0, 1}, // 数字3
 		{0, 1, 0, 1, 1, 1, 0}, // 数字4
 		{1, 1, 0, 1, 0, 1, 1}, // 数字5
 		{1, 1, 1, 1, 0, 1, 1}, // 数字6
 		{0, 1, 0, 0, 1, 0, 1}, // 数字7
 		{1, 1, 1, 1, 1, 1, 1}, // 数字8
 		{1, 1, 0, 1, 1, 1, 1}, // 数字9
};

static uint8_t SegLcd_currentCom = 0;


void SegLcd_Init(void){
	// com 1~10: B0 ~ B9
	// BIAS: D9
	// seg 1~16: E0 ~ E15
	//     17~20: B12 ~ B15
	//     21~26: D3 ~ D8
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	// 配置 bias 输出
	GPIO_InitStruct.Pin = GPIO_PIN_9;  // bit 9
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	// 配置com为输入
	GPIO_InitStruct.Pin = 0x3FF;  // bit 0~9
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// 配置seg为输入 前16位
	GPIO_InitStruct.Pin = GPIO_PIN_All;  // bit 0~15
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;  // 17 ~ 20
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;  // 21 ~ 26
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	for(uint32_t i = 0; i < 20; i++){
		SegLcd_GRAM[i] = 0;
	}
}

void SegLcd_selCom(uint8_t com){
	// 驱动方式：
	// 正周期时, 选中的 com => GND, 未选中的 com => input, bias => VCC;
	// 		此时，COM = 0V, non-COM=2.5;
	//      点亮的seg => VCC   (0 - 3.3  = -3.3),     与未选中的COM (2.5 - 3.3 = -0.8)
	//      熄灭的seg => input (0 - 1.65 = -1.65),    与未选中的COM (2.5 - 1.65 = 0.85)

	// 负周期时, 选中的 com => VCC, 未选中的 com => input, bias => GND;
	//      此时，COM = 3.3V, non-COM=0.83;
	//      点亮的seg =>   GND(3.3 - 0    = 3.3V),    与未选中的COM (0.83 - 0 = 0.83)
	//      熄灭的seg => input(3.3 - 1.65 = 1.65),    与未选中的COM (0.83 - 1.65 = -0.82)

	// com 0~19。当其大于等于10的时候，为负周期
	// 未选中的 com 为input, 选中的是 output, 只是输出电平不一样
	// 这一步将10个com都设置为输入，MODER寄存器两个位控制一个引脚。0为输入，1为输出，2复用，3模拟
	// 一个十六进制数即4个二进制位，五个0控制20个二进制位，将B0~B9总共10个引脚设置为输入

	// BSRR: Bit Set Reset Register 高16位BRY (Reset), 低16位BSY (Set)。
	// 只能以16bit为单位写入，若同时操作对应bit，BSY起作用
	GPIOB->MODER &= 0xFFF00000;
	if (com < 10){  // 正周期
		GPIOD->BSRR = 1 << 9;  // BIAS VCC
		GPIOB->BSRR = 0x3FF << 16;  // COM GND
	} else {
		GPIOD->BSRR = (1 << 9) << 16;  // BIAS GND
		GPIOB->BSRR = 0x3FF;  // COM VCC
	}
	GPIOB->MODER |= (uint32_t)1U << ((com % 10) * 2);  // set output
}
void SegLcd_selSeg(uint8_t com){
	// 处理前16bit
	uint32_t mode = 0;
	uint8_t colIndex = com % 10;
	uint16_t colMask = (uint32_t)1 << colIndex;
	for (uint8_t row = 0; row < 16; row++){
		if ((SegLcd_GRAM[SegLcd_GRAMOffset + row] & colMask) != 0){
			mode |= 1 << (row * 2);
		}
	}
	GPIOE->MODER = mode;
	if (com < 10){
		GPIOE->ODR = 0XFFFF;
	} else {
		GPIOE->ODR = 0;
	}

	// 处理后4bit B12~B15
	mode = 0;
	for (uint8_t offset = 0; offset < 4; offset++){
		if ((SegLcd_GRAM[SegLcd_GRAMOffset + 16 + offset] & colMask) != 0){
			mode |= (uint32_t)1 << ((12 + offset) * 2);  // 选中，配置为输出
		}
	}
	GPIOB->MODER &= 0x00FFFFFF;
	GPIOB->MODER |= mode;
	if (com < 10) {
		GPIOB->BSRR = 0xF000;
	} else {
		GPIOB->BSRR = 0xF000 << 16;
	}

	// 处理其它 seg
	uint16_t otherSeg = 0;
	uint8_t mainNum[4] = {
			SegLcd_TopCnt % 10,
			SegLcd_TopCnt /10 % 10,
			SegLcd_TopCnt /100 % 10,
			SegLcd_TopCnt /1000 % 10,
	};

	if (colIndex == 0){
		otherSeg |= topNumMap[mainNum[0]][0];
		if ((SegLcd_Body & (1 << 1)) != 0) {
			otherSeg |= (1 << 1);
		}
		if ((SegLcd_Body & (1 << 2)) != 0) {
			otherSeg |= (1 << 2);
		}
	} else if (colIndex == 1) {
		otherSeg |= topNumMap[mainNum[0]][1];

		if ((SegLcd_Body & (1 << 0)) != 0) {
			// proc body
			otherSeg |= (1 << 1);
		}
	} else if (colIndex == 2) {
		otherSeg |= topNumMap[mainNum[1]][0];
		if (SegLcd_SpeedMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Speed % 10][colIndex - 2];  // proc speed
		}
		if (SegLcd_LevelMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Level % 10][colIndex - 2] << 1;  // proc level
		}
		if (SegLcd_ShowMusic != 0) {
			otherSeg |= 1 << 2;
		}
	} else if (colIndex == 3) {
		otherSeg |= topNumMap[mainNum[1]][1];
		if (SegLcd_SpeedMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Speed % 10][colIndex - 2];  // proc speed
		}
		if (SegLcd_LevelMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Level % 10][colIndex - 2] << 1;  // proc level
		}
	} else if (colIndex == 4) {
		if (SegLcd_TopCntMode == 0) {
			otherSeg |= topNumMap[mainNum[2]][0];
		}
		if (SegLcd_SpeedMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Speed % 10][colIndex - 2];  // proc speed
		}
		if (SegLcd_LevelMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Level % 10][colIndex - 2] << 1;  // proc level
		}
	} else if (colIndex == 5) {
		if (SegLcd_TopCntMode == 0) {
			otherSeg |= topNumMap[mainNum[2]][1];
		}else if (SegLcd_TopCntMode == 2) {
			otherSeg |= 0b100000;
		}
		if (SegLcd_SpeedMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Speed % 10][colIndex - 2];  // proc speed
		}
		if (SegLcd_LevelMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Level % 10][colIndex - 2] << 1;  // proc level
		}
	} else if (colIndex == 6) {
		if (SegLcd_TopCntMode == 0 || SegLcd_TopCntMode == 2) {
			otherSeg |= topNumMap[mainNum[3]][0];
		}
		if (SegLcd_SpeedMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Speed % 10][colIndex - 2];  // proc speed
		}
		if (SegLcd_LevelMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Level % 10][colIndex - 2] << 1;  // proc level
		}
	} else if (colIndex == 7) {
		if (SegLcd_TopCntMode == 0 || SegLcd_TopCntMode == 2) {
			otherSeg |= topNumMap[mainNum[3]][1];
		}
		if (SegLcd_SpeedMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Speed % 10][colIndex - 2];  // proc speed
		}
		if (SegLcd_LevelMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Level % 10][colIndex - 2] << 1;  // proc level
		}
	} else if (colIndex == 8) {
		// process SegLcd_PrevBox
		for(int8_t i = 0; i < 4; i++){
			if ((SegLcd_PrevBox & (1 << i)) != 0) {
				otherSeg |= PrevBoxMap[0][i];
			}
		}
		if (SegLcd_SpeedMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Speed % 10][colIndex - 2];  // proc speed
		}
		if (SegLcd_LevelMode == 0) {
			otherSeg |= SpeedMap[SegLcd_Level % 10][colIndex - 2] << 1;  // proc level
		}
	} else if (colIndex == 9) {
		for(int8_t i = 0; i < 4; i++){
			if ((SegLcd_PrevBox & (1 << (i + 4))) != 0) {
				otherSeg |= PrevBoxMap[1][i];
			}
		}
		otherSeg |= SegLcd_Speed >= 10 ? 1 : 0;
		otherSeg |= SegLcd_Level >= 10 ? 2 : 0;
	}

	uint32_t finalMode = 0;
	for (uint8_t testBit = 0; testBit < 6; testBit++){
		if ((otherSeg & (1 << testBit)) != 0){
			finalMode |= (uint32_t)1 << ((3 + testBit)*2);
		}
	}
	GPIOD->MODER &= ~0x0003FFC0;  // bit 3 ~ 8 set to input
	GPIOD->MODER |= finalMode;  // set output
	if (com < 10){
		GPIOD->BSRR = 0b111111000;//  (uint32_t)otherSeg << 3;
	} else {
		GPIOD->BSRR = 0b111111000 << 16; // ((uint32_t)otherSeg << 3) << 16;
	}
}

void SegLcd_DrawOneFrame(void){
	SegLcd_TickMs++;

	SegLcd_selCom(SegLcd_currentCom);
	SegLcd_selSeg(SegLcd_currentCom);

	SegLcd_currentCom += 1;
	SegLcd_currentCom %= 20;
}

void SegLcd_SetBGLight(uint8_t status){
	static uint8_t old = 3;
	if (old == status) {
		return;
	}
	old = status;
	HAL_GPIO_WritePin(SEGLCD_BG_LED_GPIO_Port, SEGLCD_BG_LED_PIN, status == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void SegLcd_SetPrevBox(u8 data) {
	SegLcd_PrevBox = data;
}

void SegLcd_SetTopValue(u16 data, u8 mode) {
	SegLcd_TopCnt = data;
	SegLcd_TopCntMode = mode;
}

void SegLcd_SetLevelValue(u8 data, u8 mode) {
	SegLcd_Level = data;
	SegLcd_LevelMode = mode;
}

u64 SegLcd_GetTick() {
	return SegLcd_TickMs;
}
