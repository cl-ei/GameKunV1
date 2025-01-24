#include "sd_driver.h"
#include "debug.h"

extern SD_HandleTypeDef hsd1;
#define opHsd hsd1

#define SD_TIMEOUT                  ((uint32_t)0xFFFFFFFFU)

HAL_SD_CardInfoTypeDef CL_pCardInfo;
static uint8_t CL_sdIsTransmiting = 0;
static uint8_t SD_Inited = 0;

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd) {
	print("sd err!");
	CL_sdIsTransmiting = 0;
	if(hsd->State != HAL_SD_STATE_ERROR){
		hsd->State = HAL_SD_STATE_ERROR;
	}
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd){
	CL_sdIsTransmiting = 0;
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd){
	CL_sdIsTransmiting = 0;
}

int8_t CL_SD_Init(void) {
	if (SD_Inited) {
		return 0;
	}
	SD_Inited = 1;

	hsd1.Instance = SDMMC1;
	hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
	hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
	hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
	hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
	hsd1.Init.ClockDiv = 0;
	return HAL_SD_Init(&opHsd);
}
int8_t CL_SD_DeInit(void) {
	SD_Inited = 0;
	return HAL_SD_DeInit(&opHsd);
}

HAL_StatusTypeDef CL_getSDState() {
	HAL_SD_CardStateTypeDef status;
	status = HAL_SD_GetCardState(&opHsd);
	if (status == HAL_SD_CARD_ERROR) {
		print("sd error status: %x\n", status);
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef CL_readSdInfo(){
	HAL_SD_CardCIDTypedef CL_pCID;
	HAL_SD_CardCSDTypedef CL_pCSD;
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_SD_GetCardCID(&opHsd, &CL_pCID);
	if (status != HAL_OK){
		print("cid read err: %x\n", status);
		return status;
	}
	status = HAL_SD_GetCardCSD(&opHsd, &CL_pCSD);
	if (status != HAL_OK){
		print("csd read err: %x\n", status);
		return status;
	}
	status = HAL_SD_GetCardInfo(&opHsd, &CL_pCardInfo);
	if (status != HAL_OK){
		print("card info read err: %x\n", status);
		return status;
	}
	CL_pCardInfo.BlockNbr = CL_pCardInfo.BlockNbr;
	return HAL_OK;
}

HAL_StatusTypeDef CL_sdReadBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks){
	HAL_StatusTypeDef status;
	uint32_t start_time;

	// 等待SD状态由忙转为就绪态
	start_time = HAL_GetTick();
	while(HAL_SD_GetCardState(&opHsd) != HAL_SD_CARD_TRANSFER) {
		if ((HAL_GetTick() - start_time) >= SD_TIMEOUT) {
			return HAL_ERROR;
		}
	}

	// 启动传输
	CL_sdIsTransmiting = 1;
	status = HAL_SD_ReadBlocks_DMA(&opHsd, pData, BlockAdd, NumberOfBlocks);
	if (status != HAL_OK) {
		return status;
	}

	// 启动DMA成功，等待读完成。在DMA完成中断HAL_SD_RxCpltCallback中，会将CL_sdIsTransmiting置为0
	start_time = HAL_GetTick();
	while(CL_sdIsTransmiting == 1) {
		if((HAL_GetTick() - start_time) >= SD_TIMEOUT) {
			return HAL_ERROR;
		}
	}
	return HAL_OK;
}

HAL_StatusTypeDef CL_sdWriteBlocks(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks){
	HAL_StatusTypeDef status;
	uint32_t start_time;

	// 等待SD状态由忙转为就绪态
	start_time = HAL_GetTick();
	while(HAL_SD_GetCardState(&opHsd) != HAL_SD_CARD_TRANSFER) {
		if ((HAL_GetTick() - start_time) >= SD_TIMEOUT) {
			return HAL_ERROR;
		}
	}

	// 启动DMA传输
	CL_sdIsTransmiting = 1;
	status = HAL_SD_WriteBlocks_DMA(&opHsd, pData, BlockAdd, NumberOfBlocks);
	if (status != HAL_OK) {
		return status;
	}

	// 启动DMA成功，等待读完成。在DMA完成中断HAL_SD_TxCpltCallback中，会将CL_sdIsTransmiting置为0
	start_time = HAL_GetTick();
	while(CL_sdIsTransmiting == 1) {
		if((HAL_GetTick() - start_time) >= SD_TIMEOUT) {
			return HAL_ERROR;
		}
	}

	return HAL_OK;
}
