#ifndef __ST_LOG_H__
#define __ST_LOG_H__
#include "STOSConfig.h"

#if defined __ST_OS_ANDROID__
#include <android/log.h>
#include <string.h>

#define  LOG_TAG    "Player_2"

#define  STLOGI(...) (__android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__))
#define  STLOGE(...) (__android_log_print(ANDROID_LOG_ERROR,LOG_TAG, __VA_ARGS__))
#define  STLOGW(...) (__android_log_print(ANDROID_LOG_ERROR,LOG_TAG, __VA_ARGS__))
#define  STLOGV(...) __android_log_print
#elif defined __ST_OS_WINDOWS__
#define  STLOGI(...)  printf(__VA_ARGS__)
#define  STLOGE(...)  printf(__VA_ARGS__)
#define  STLOGW(...)  printf(__VA_ARGS__)
#define  STLOGV(...) printf(__VA_ARGS__)
#else
#define  STLOGI(...)
#define  STLOGE(...)
#define  STLOGW(...)
#define  STLOGV(...)
#endif

#endif
