#include "maudio.h"

#include "custom/ff15/ff.h"
#include "custom/logic/mevent.h"
#include "custom/cxk.h"

float gain = (0.04);
u8 isPlaying = 0;

FATFS fs;
FRESULT fr;
FIL tmpFile;
DIR dir;
FILINFO fno;
UINT br = 0;

// #define AUDIO_BUFF_LEN 最后的字节数不能超过65535, 否则DMA一次发不完
#define AUDIO_BUFF_LEN (1024*10)
uint16_t audioBuff[AUDIO_BUFF_LEN] = {};

extern I2C_HandleTypeDef hi2c2;

i8 Audio_Init() {
	fr = f_mount(&fs, "0:", 1);
	print("mount fs r: %d", fr);
	if (fr != FR_OK) {
		return -1;
	}
	return 0;
}

u32 Audio_SongTime = 0;
u8 Audio_SongIndex = 0;

i8 Audio_GetSongCount() {
	i8 count = 0;

	fr = f_opendir(&dir, "");
	if (fr != FR_OK) {
		print("open dir failed: %d", fr);
		return -1;
	}
	while (count < 100) {
		fr = f_readdir(&dir, &fno);
		print("read dir...");

		if (fr != FR_OK || fno.fname[0] == 0) {
			// 读取目录结束或出错，跳出循环
			print("read finished, fr: %d", fr);
			break;
		}

		// ensure postfix .wav
		print("fname: %s", fno.fname);
		for (uint32_t i = 0; i < 200; i++) {
			if (fno.fname[i] == '.' && fno.fname[i + 1] == 'w' && fno.fname[i + 2] == 'a') {
				count += 1;
				break;
			}
		}
	}
	fr = f_closedir(&dir);
	print("close dir fr: %d", fr);
	return count;
}

void Audio_RenderSongTitle() {
	Scr_Clean();

	i16 size = 0;
	for (u8 i = 0; i < 50; i++) {
		if (fno.fname[i] == 0) {
			break;
		}
		size = i + 1;
	}
	renderText(fno.fname, size - 4);  // 去掉.wav
}

int8_t Audio_Start(i8 index) {
	uint8_t tmpBuff[100];
	i8 songIdx = -1;

	fr = f_opendir(&dir, "");
	if (fr != FR_OK) {
		print("open dir failed: %d", fr);
		return -1;
	}

	while (songIdx < index) {
		fr = f_readdir(&dir, &fno);
		print("read dir...");

		if (fr != FR_OK || fno.fname[0] == 0) {
			// 读取目录结束或出错，跳出循环
			print("read finished, fr: %d", fr);
			break;
		}

		// ensure postfix .wav
		print("fname: %s", fno.fname);
		for (uint32_t i = 0; i < 200; i++) {
			if (fno.fname[i] == '.' && fno.fname[i + 1] == 'w' && fno.fname[i + 2] == 'a') {
				songIdx += 1;
				break;
			}
		}
	}
	fr = f_closedir(&dir);
	print("close dir fr: %d", fr);

	fr = f_open(&tmpFile, fno.fname, FA_READ | FA_OPEN_EXISTING);
	if (fr != FR_OK) {
		print("open music err: %d", fr);
		return -1;
	}
	fr = f_read(&tmpFile, (void *)tmpBuff, 100, &br); // 抛弃最起始的100个字节
	if (fr != FR_OK) {
		print("read err: %d", fr);
		return -1;
	}

	print("read succ.");
	TAS2563_Init(audioBuff, AUDIO_BUFF_LEN * 2);
	isPlaying = 1;
	Audio_SongTime = 0;
	Audio_SongIndex = index;
	Audio_RenderSongTitle();
	return 0;
}

void Audio_Pause() {
	isPlaying = 0;
	TAS2563_Stop();
}

void Audio_Play() {
	isPlaying = 1;
	TAS2563_Init(audioBuff, AUDIO_BUFF_LEN * 2);
}

int8_t Audio_Stop() {
	isPlaying = 0;
	TAS2563_Stop();
	fr = f_close(&tmpFile);
	if (fr != 0) {
		print("audio stop error: %d", fr);
	}
	return 0;
}

void Audio_LoadBuff() {
	uint8_t flag = TAS2563_GetAndClearFlag();
	if (flag == 1) {
		// 填充前半部分

		fr = f_read(&tmpFile, (void *)audioBuff, AUDIO_BUFF_LEN, &br);
		if (fr != 0 || br == 0){
			print("half read res: %x, br: %ld", fr, br);
			TAS2563_Stop();
			f_close(&tmpFile);
			isPlaying = 0;
			Event_Add(EVENT_MUSIC_END);
		}

		// 写入到 audioBuff 如果是SAI 需要填充；I2S不需要
		for (uint32_t i = 0; i < AUDIO_BUFF_LEN/2; i++){
			audioBuff[i] = (int16_t)(((int16_t)(audioBuff[i]))*gain);
		}
		Audio_SongTime += 55;
	} else if (flag == 2) {
		// 填充后半部分

		fr = f_read(&tmpFile, (void *)(&(audioBuff[AUDIO_BUFF_LEN/2])), AUDIO_BUFF_LEN, &br);
		if (fr != 0 || br == 0){
			print("complete read res: %x, br: %ld", fr, br);
			TAS2563_Stop();
			f_close(&tmpFile);
			isPlaying = 0;
			Event_Add(EVENT_MUSIC_END);
		}

		// 写入到 audioBuff
		for (uint32_t i = AUDIO_BUFF_LEN/2; i < AUDIO_BUFF_LEN; i++){
			audioBuff[i] = (int16_t)(((int16_t)(audioBuff[i]))*gain);
		}
		Audio_SongTime += 55;
	}
}

u8 Audio_IsPlaying() {
	return isPlaying;
}

const float volumeMap[11] = {0, 0.01, 0.06, 0.10, 0.18, 0.28, 0.36, 0.46, 0.54, 0.64, 0.75};

void Audio_SetVol(u8 vol) {
	if (vol > 10) {
		vol = 10;
	}
	gain = volumeMap[vol];
}

// play control
extern uint16_t SegLcd_GRAM[20*200];
uint32_t frameIndex = 0;
u32 renderMp3Tick = 0;

void Audio_RenderPlayTime() {
	static u32 oldSongTime = 0;

	if (Audio_SongTime == oldSongTime) {
		return;
	}
	oldSongTime = Audio_SongTime;
	SegLcd_SetTopValue((oldSongTime / 1000 / 60 * 1000) + ((oldSongTime/1000) % 60), 2);
}

void Audio_RenderUI(u32 tick) {
	Audio_RenderPlayTime();

	if (tick - renderMp3Tick < 130) {
		return;
	}
	renderMp3Tick = tick;

	Scr_ShowAndScroll();

	if (Audio_SongTime < 3000) {
		// render index
		for (i8 i = 12; i < 20; i++) {
			SegLcd_GRAM[i] = 0;
		}
		u8 b = ((Audio_SongIndex + 1) / 10) % 10;
		u8 g = (Audio_SongIndex + 1) % 10;

		copyNumToGram(1, 14, b);
		copyNumToGram(5, 14, g);
	}
	if (isPlaying) {
		 flushProgresBar();
	}

	/*
	uint16_t ct = 0;
	u32 st = 0;

	for (uint8_t y = 0; y < 12; y ++) {
		ct = cxk_data[frameIndex];
		frameIndex++;
		SegLcd_GRAM[y] = ~ct;
	}

	if (frameIndex >= 17580) {
		frameIndex = 0;
	}
	*/
}

