#include "tas2563.h"
#include "debug.h"

extern I2C_HandleTypeDef hi2c2;
#define H_TAS_I2C hi2c2

//#define USE_SAI

#ifdef USE_SAI
    extern SAI_HandleTypeDef hsai_BlockA1;
    #define H_TAS_SAI hsai_BlockA1
#else
    extern I2S_HandleTypeDef hi2s1;
    #define H_TAS_I2S hi2s1
    // DMA_HandleTypeDef hdma_spi1_tx;
#endif

int8_t saiTxFlag = 0;

int8_t TAS2563_Init(uint8_t *buff, uint32_t len) {
	// I2C
	uint8_t data = 0;
	int8_t state;
	print("start init tas2563");

	// 开始SAI 信号输送
	memset((void *)buff, 0, len);
#ifdef USE_SAI
	HAL_SAI_Transmit_DMA(&H_TAS_SAI, buff, (uint16_t)(len / 4));  // 因为DMA一次发送word,因此需要除以4
#else
	state = HAL_I2S_Transmit_DMA(&H_TAS_I2S, buff, (uint16_t)(len / 2));  // size 是16bit的数据的长度。
	if (state != HAL_OK) {
		print("init i2s error: %d", state);
		return state;
	}
#endif
	state = HAL_I2C_Mem_Read(&H_TAS_I2C, 0x48 << 1, 0x02, I2C_MEMADD_SIZE_8BIT, &data, 1, 0xFFFFFF);

	data = 0;
	state = HAL_I2C_Mem_Write(&H_TAS_I2C, (0x4c << 1) | 1, 0x02, I2C_MEMADD_SIZE_8BIT, &data, 1, 0xFF);

	state = HAL_I2C_Mem_Read(&H_TAS_I2C, 0x4c << 1, 0x02, I2C_MEMADD_SIZE_8BIT, &data, 1, 0xFFFFFFFF);
	state = HAL_I2C_Mem_Read(&H_TAS_I2C, 0x4c << 1, 0x03, I2C_MEMADD_SIZE_8BIT, &data, 1, 0xFFFFFFFF);

	data = 16 << 1;
	state = HAL_I2C_Mem_Write(&H_TAS_I2C, (0x4c << 1) | 1, 0x03, I2C_MEMADD_SIZE_8BIT, &data, 1, 0xFFFFFFFF);
	print("tas init complete. state: %X, data: %02X", state, data);
	return 0;
}


void TAS2563_Stop() {
#ifdef USE_SAI
	HAL_SAI_DMAStop(&H_TAS_SAI);
#else
	HAL_I2S_DMAStop(&H_TAS_I2S);
#endif
	print("tas2563 stopped.");
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	saiTxFlag = 1;
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
	saiTxFlag = 2;
}

int8_t TAS2563_GetAndClearFlag() {
	int8_t rst = saiTxFlag;
	if (rst != 0) {
		saiTxFlag = 0;
	}
	return rst;
}

