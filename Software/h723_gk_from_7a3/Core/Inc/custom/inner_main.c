#include "inner_main.h"
#include "stm32h7xx_hal.h"
#include "debug.h"

#include "usb_device.h"
#include "usbd_msc.h"
#include "usbd_desc.h"
#include "usbd_storage_if.h"

#include "custom/ff15/ff.h"

#include "tas2563.h"
#include "logic/display.h"
#include "logic/mevent.h"
#include "logic/maudio.h"

extern TIM_HandleTypeDef htim3;
extern RTC_HandleTypeDef hrtc;


u8 sysHour = 0;
u8 sysMin = 0;
u8 sysSec = 0;
u16 sysMicSec = 0;
u8 sysBtnTick = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM3){
		// every 1ms
		sysMicSec++;
		if (sysMicSec >= 1000) {
			sysMicSec = 0;
			sysSec++;
			if (sysSec >= 60) {
				sysSec = 0;
				sysMin++;
				if (sysMin >= 60) {
					sysMin=0;
					sysHour++;
					if (sysHour >= 24) {
						sysHour = 0;
					}
				}
			}
		}

		// check button
		sysBtnTick++;
		if (sysBtnTick >= 3) {
			sysBtnTick = 0;
			Event_CheckButton();
		}

		// flush lcd
		SegLcd_DrawOneFrame();
	}
}

// USB
extern USBD_HandleTypeDef hUsbDeviceHS;

i8 CL_USB_DEVICE_Init(void) {
	if (USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK) {
		return -1;
	}
	if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_MSC) != USBD_OK) {
		return -1;
	}
	if (USBD_MSC_RegisterStorage(&hUsbDeviceHS, &USBD_Storage_Interface_fops_HS) != USBD_OK) {
		return -1;
	}

	if (USBD_Start(&hUsbDeviceHS) != USBD_OK) {
		return -1;
	}
	HAL_PWREx_EnableUSBVoltageDetector();
	return 0;
}


i8 CL_USB_Device_DeInit() {
	HAL_PWREx_DisableUSBVoltageDetector ();

	if (USBD_Stop(&hUsbDeviceHS) != 0) {
		return -1;
	}
	return USBD_DeInit(&hUsbDeviceHS);
}

i8 SetSysFreq(u8 isHigh) {
	u8 rst = 0;

	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
	if (isHigh) {
		__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
	} else {
		__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
	}

	while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

	// config pll
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
								  |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

	if (isHigh) {
	  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	} else {
	  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV16;
	}

	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
	if (isHigh) {
		rst = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
	} else {
		rst = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
	}
	if (rst != HAL_OK) {
		print("sys clk config error: %d", rst);
		return -1;
	}
	print("sys clk config ok.");

	// 需要设置 TIM3的频率
	HAL_TIM_Base_Stop_IT(&htim3);
	HAL_TIM_Base_DeInit(&htim3);

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim3.Instance = TIM3;
	if (isHigh) {
		htim3.Init.Prescaler = 240-1;
	} else {
		htim3.Init.Prescaler = 15-1;
	}

	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 1000;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	rst = HAL_TIM_Base_Init(&htim3);
	if (rst != HAL_OK) {
		print("tim error: %d", rst);
		return -1;
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	rst = HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);
	if (rst != HAL_OK) {
		print("tim clk config error: %d", rst);
		return -1;
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	rst = HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);
	if (rst != HAL_OK) {
		print("tim master config result: %d", rst);
	    return -1;
	}
	HAL_TIM_Base_Start_IT(&htim3);
	return 0;
}

// mode 0: rest
// 1. play
#define PLAYMODE_MAIN_PAGE  0
#define PLAYMODE_MP3        1
#define PLAYMODE_HOUR_SET_PAGE  2
#define PLAYMODE_MIN_SET_PAGE  3
#define PLAYMODE_USB_PAGE  4

u32 playMode = PLAYMODE_MAIN_PAGE;
u8 Mp3_SongCount = 0;
u8 Mp3_SongIndex = 0;
u8 Mp3_Volume = 2;

void Mp3_SetVol() {
	SegLcd_SetLevelValue(Mp3_Volume, 0);
	Audio_SetVol(Mp3_Volume);
}
extern u8 SegLcd_ShowMusic;
extern u8 SegLcd_Body;
extern u8 SegLcd_PrevBox;

i8 Mp3_Enter() {
	SegLcd_ShowMusic = 1;
	SegLcd_Body = 0;

	u8 result = Audio_Init();
	print("audio init result: %d", result);
	if (result != 0) {
		return result;
	}

	Mp3_SongCount = Audio_GetSongCount();
	SegLcd_SetTopValue(0, 2);
	result = Audio_Start(Mp3_SongIndex);
	if (result == 0) {
		Mp3_SetVol();
	}
	return result;
}

i8 Mp3_Exit() {
	SegLcd_ShowMusic = 0;
	SegLcd_Body = 0b101;
	SegLcd_PrevBox = 0;

	Scr_Clean();
	Scr_RenderTime(88, 88, 88); // 清除缓存，下次渲染刷新真实值
	SegLcd_SetLevelValue(Mp3_Volume, 1);
	i8 result = Audio_Stop();
	if (result != 0) {
		print("stop audio error: %d", result);
		return result;
	}
	return result;
}


u8 uartTxBuff[65];
u8 uartRxBuff[64];
u8 wifiConfLen = 0;
extern UART_HandleTypeDef huart2;

// 0 准备开机设置时间; 1. 等待m5开机; 2 发送并进入3; 3 检查时间，如果超过1个小时，则进入0 开始重新校准
u8 m5Status = 0;
u32 m5Tick = 0;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	if(huart == (&huart2)) {
		print("s: %d", Size);
		if (Size == 3 &&
			(uartRxBuff[0] & 0xC0) == 0xC0 &&
			(uartRxBuff[1] & 0x80) == 0x80 &&
			(uartRxBuff[2] & 0x40) == 0x40) {

			sysHour = uartRxBuff[0] & (~0xC0);
			sysMin = uartRxBuff[1] & (~0x80);
			sysSec = uartRxBuff[2] & (~0x40);
			m5Status = 4;
		}
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uartRxBuff, 32);
	}
}

void CheckNetTime(u32 tick) {
	u8 rst = 0;

	// 准备
	if (m5Status == 0) {
		m5Status = 1;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
		m5Tick = tick;
	} else if (m5Status == 1) {
		if ((tick - m5Tick) > 5000) {
			m5Status = 2;
		}
	} else if (m5Status == 2) {
		for (i8 i = 0; i < 8; i++) {
			rst = HAL_UART_Receive(&huart2, uartRxBuff, 32, 0x1);
			print("receive rst: %d");
			if (rst != 0) {
				break;
			}
		}
		rst = HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uartRxBuff, 32);
		print("set idle: %d", rst);

		rst = HAL_UART_Transmit(&huart2, uartTxBuff, wifiConfLen, 0xFFFF);
		print("wifi conf tx: %d, len: %d", rst, wifiConfLen);

		m5Status = 3;
	} else if (m5Status == 3) {
	} else if (m5Status == 4) {
		// close m5
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);

		RTC_TimeTypeDef realTime;
		realTime.Hours = sysHour;
		realTime.Minutes = sysMin;
		realTime.Seconds = sysSec;
		u8 rst = HAL_RTC_SetTime(&hrtc, &realTime, RTC_FORMAT_BIN);
		print("set ntp rst: %d", rst);

		m5Status = 5;
	} else if (m5Status == 5) {
		if ((tick - m5Tick) > (1000 * 7200)) {
			m5Status = 0;
		}
	}
}

i8 loadWifiPass() {
	FIL tmpFile;
	FRESULT fr;
	UINT br = 0;
	TCHAR fno[9] = {'w', 'i', 'f', 'i', '.', 't', 'x', 't', '\0'};

	u8 rst = Audio_Init();
	if (rst != 0) {
		print("mount err: %d", rst);
		return rst;
	}
	fr = f_open(&tmpFile, fno, FA_READ | FA_OPEN_EXISTING);
	if (fr != FR_OK) {
		print("open wifi conf err: %d", fr);
		return -1;
	}
	fr = f_read(&tmpFile, (void *)uartTxBuff, 63, &br); // 抛弃最起始的100个字节
	if (fr != FR_OK) {
		print("read err: %d", fr);
		return -1;
	}
	print("read br: %d", br);
	wifiConfLen = (br >= 63) ? 63 : br;
	uartTxBuff[64] = 0;
	print(uartTxBuff);
	return 0;
}

void CheckBgLight(u32 tick) {
	static u32 bgLightTick = 0;
	if (tick - bgLightTick > 200) {
		bgLightTick = tick;
		SegLcd_SetBGLight((Event_GetLastEventDur() < 15000) ? 1 : 0);
	}
}

u8 EventLoopMainPage() {
	u32 tick;
	u8 rst;
	u8 event;
	u8 retMode = 0;

	Scr_Clean();
	Scr_RenderTime(88, 88, 88); // 清除缓存，下次渲染刷新真实值

	rst = SetSysFreq(0);
	print("set low pwr rst: %d", rst);

	// update time
	RTC_TimeTypeDef realTime;
	RTC_DateTypeDef sDate;
	rst = HAL_RTC_GetTime(&hrtc, &realTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	sysHour = realTime.Hours;
	sysMin = realTime.Minutes;
	sysSec = realTime.Seconds;

	while (1) {
		tick = HAL_GetTick();
		CheckBgLight(tick);
		event = Event_Get();

		CheckNetTime(tick);
		Scr_RenderTime(sysHour, sysMin, sysSec);

		if (event == EVENT_KEY_SPIN_LONG) {
			rst = Mp3_Enter();
			if (rst == 0) {
				return PLAYMODE_MP3;
			} else {
				print("process error: %d", rst);
			}
		} else if (event == EVENT_KEY_RST_LONG) {
			return PLAYMODE_HOUR_SET_PAGE;
		} else if (event == EVENT_KEY_START_LONG) {
			return PLAYMODE_USB_PAGE;
		}
	}
}

u8 EventLoopMP3() {
	u32 tick;
	u8 rst;
	u8 event;

	rst = SetSysFreq(1);
	print("set high pwr rst: %d", rst);
	rst = Mp3_Enter();
	if (rst != 0) {
		return PLAYMODE_MAIN_PAGE;
	}
	while (1) {
		tick = HAL_GetTick();
		CheckBgLight(tick);
		event = Event_Get();

		Audio_RenderUI(tick);
		Audio_LoadBuff();

		if (event == EVENT_KEY_SPIN) {
			if(Audio_IsPlaying()){
				Audio_Pause();
			} else {
				Audio_Play();
			}
		} else if (event == EVENT_KEY_UP) {
			if (Mp3_Volume < 10) {
				Mp3_Volume++;
			}
			Mp3_SetVol();
		} else if (event == EVENT_KEY_DOWN) {
			if (Mp3_Volume > 0) {
				Mp3_Volume--;
			}
			Mp3_SetVol();
		} else if (event == EVENT_KEY_LEFT) {
			Audio_Stop();
			if (Mp3_SongIndex == 0) {
				Mp3_SongIndex = Mp3_SongCount - 1;
			} else {
				Mp3_SongIndex--;
			}
			Audio_Start(Mp3_SongIndex);
			SegLcd_SetTopValue(Mp3_SongCount*100 + Mp3_SongIndex + 1, 0);
		} else if (event == EVENT_KEY_RIGHT || event == EVENT_MUSIC_END) {
			Audio_Stop();
			Mp3_SongIndex++;
			if (Mp3_SongIndex >= Mp3_SongCount) {
				Mp3_SongIndex = 0;
			}
			Audio_Start(Mp3_SongIndex);
			SegLcd_SetTopValue(Mp3_SongCount*100 + Mp3_SongIndex + 1, 0);
		} else if (event == EVENT_KEY_SPIN_LONG) {
			rst = Mp3_Exit();
			if (rst == 0) {
				return PLAYMODE_MAIN_PAGE;
			} else {
				print("exit mp3 error: %d", rst);
			}
		}
	}
}

u8 EventLoopHourSet() {
	u32 tick;
	u8 event;

	SetSysFreq(0);

	while (1) {
		tick = HAL_GetTick();
		CheckBgLight(tick);
		event = Event_Get();

		Scr_FlashHourMin(tick, sysHour, sysMin, sysSec, 0);
		if (event == EVENT_KEY_RST) {
			return PLAYMODE_MIN_SET_PAGE;
		} else if (event == EVENT_KEY_UP) {
			sysHour = sysHour == 23 ? 0 : (sysHour + 1);
		} else if (event == EVENT_KEY_DOWN) {
			sysHour = sysHour == 0 ? 23 : (sysHour - 1);
		}
	}
}

u8 EventLoopMinSet() {
	u32 tick;
	u8 event;
	u8 rst;

	SetSysFreq(0);

	while (1) {
		tick = HAL_GetTick();
		CheckBgLight(tick);
		event = Event_Get();

		Scr_FlashHourMin(tick, sysHour, sysMin, sysSec, 1);

		if (event == EVENT_KEY_RST) {
			RTC_TimeTypeDef realTime;
			realTime.Hours = sysHour;
			realTime.Minutes = sysMin;
			realTime.Seconds = sysSec;
			rst = HAL_RTC_SetTime(&hrtc, &realTime, RTC_FORMAT_BIN);
			if (rst != 0) {
				print("set failed: %d", rst);
			} else {
				return PLAYMODE_MAIN_PAGE;
			}
		} else if (event == EVENT_KEY_UP) {
			sysMin = sysMin == 59 ? 0 : (sysMin + 1);
		} else if (event == EVENT_KEY_DOWN) {
			sysMin = sysMin == 0 ? 59 : (sysMin - 1);
		}
	}
}

u8 EventLoopUSB() {
	u32 tick;
	u8 event;

	SetSysFreq(1);
	CL_USB_DEVICE_Init();

	Scr_RenderUSBPage();

	while (1) {
		tick = HAL_GetTick();
		CheckBgLight(tick);
		event = Event_Get();

		if (event == EVENT_KEY_START_LONG) {
			CL_USB_Device_DeInit();
			return PLAYMODE_MAIN_PAGE;
		}
	}
}

void myMain() {
	uint32_t tick = 0;
	u8 event = 0;
	i8 rst = 0;
	print("enter myMain.");

	Event_Init();
	HAL_TIM_Base_Start_IT(&htim3);

	CL_SD_Init();
	rst = loadWifiPass();
	print("wifi load rst: %d", rst);

	// 开启 LCD
	SegLcd_Init();
	SegLcd_SetBGLight(1);

	// 开启时钟计时
	SetSysFreq(0);
	print("enter main loop.");

	HAL_Delay(500);
	while (1) {
		if (playMode == PLAYMODE_MAIN_PAGE) {
			playMode = EventLoopMainPage();
		} else if (playMode == PLAYMODE_MP3) {
			playMode = EventLoopMP3();
		} else if (playMode == PLAYMODE_HOUR_SET_PAGE) {
			playMode = EventLoopHourSet();
		} else if (playMode == PLAYMODE_MIN_SET_PAGE) {
			playMode = EventLoopMinSet();
		} else if (playMode == PLAYMODE_USB_PAGE) {
			playMode = EventLoopUSB();
		}
	} // end of while(1)
}
