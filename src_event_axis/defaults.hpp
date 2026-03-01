#pragma once

#include <string>

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
const int CAPTURE_FPS = 10;
const std::string CAPTURE_FORMAT = "Y800";
const long BYTES_IN_MB = 1024 * 1024;
const int MAX_STATISTICS_FILE_SIZE_MB = 5;

/*** From legacy _common.h */ 
/* #define GLOBAL_UPDATE_STAT_FRAMENUM		1  //интервал (кадры) обновления глобальной статистики
/* #define GLOBAL_UPDATE_CONFIG_FRAMENUM	10 //проверки конфигурации
/*** */

/*** From legacy common/_common.h */ 
/* #define COMPENSATE_LIGHTCHANGE 1 //Включить компенсацию быстрого изменения яркости
/*
/*	#define CONNECT_TO_CAMERA_TCP_SERVER_TIMEOUT_MS		10000
/*	#define CONNECT_TO_CAMERA_COMMON_TIMEOUT_MS			3000
/*	#define CONNECT_TO_CAMERA_VIDEO_TIMEOUT_MS			50
/*	#define CONNECT_TO_CAMERA_VIDEO_TIMEOUT_NUM			300 //all time=CONNECT_TO_CAMERA_VIDEO_TIMEOUT_NUM*CONNECT_TO_CAMERA_VIDEO_TIMEOUT_MS
/*
/*	//tcpclient
/*	#define COMMAND_DELAY_MS	100
/*	#define CHAR_BUFFER_SIZE	8192
/*** */
