#include "custom/debug.h"
#include "stdarg.h"
#include "stm32h7xx_hal.h"

#define CL_FORMAT_BUFF_LEN 512
#define CL_PRINT_BUFF_LEN 1024

uint8_t CL_formatBuff[CL_FORMAT_BUFF_LEN];
uint8_t CL_printBuff[CL_PRINT_BUFF_LEN];
extern UART_HandleTypeDef huart4;

void print(const char *format,...) {
	uint16_t length;
	va_list args;

	uint16_t i;
	for(i = 0; i < CL_FORMAT_BUFF_LEN - 1; i++){
		if(*(format + i) == '\0') {
			break;
		}
		CL_formatBuff[i] = *(format + i);
	}
	CL_formatBuff[i++] = '\n';
	CL_formatBuff[i] = '\0';

	va_start(args, format);
	length = vsnprintf((char*)CL_printBuff, sizeof(CL_printBuff)+1, (char*)CL_formatBuff, args);
	va_end(args);
	HAL_UART_Transmit(&huart4, CL_printBuff, length, 0xFFFF);
}
