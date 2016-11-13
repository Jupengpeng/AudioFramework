#ifndef __TT_LOG_H__
#define __TT_LOG_H__

// INCLUDES
#include "TTOsalConfig.h"
//#define __TRACE_FILE__
#if defined (__TT_OS_ANDROID__)
#if defined (__TRACE_FILE__)
#include <stdio.h>
#define LOGI(...) {FILE* fp = fopen("/sdcard/TTPOD_TTMediaPlayer.log", "a+"); \
					fprintf(fp, __VA_ARGS__); \
					fprintf(fp, "\n"); \
					fflush(fp); \
					fclose(fp);} 
#define LOGE(...) {FILE* fp = fopen("/sdcard/TTPOD_TTMediaPlayer.log", "a+"); \
					fprintf(fp, __VA_ARGS__); \
					fprintf(fp, "\n"); \
					fflush(fp); \
					fclose(fp);} 
#define LOGW(...) {FILE* fp = fopen("/sdcard/TTPOD_TTMediaPlayer.log", "a+"); \
					fprintf(fp, __VA_ARGS__); \
					fprintf(fp, "\n"); \
					fflush(fp); \
					fclose(fp);} 
#define LOGV(...) {FILE* fp = fopen("/sdcard/TTPOD_TTMediaPlayer.log", "a+"); \
					fprintf(fp, __VA_ARGS__); \
					fprintf(fp, "\n"); \
					fflush(fp); \
					fclose(fp);} 
#define LOGD(...) {FILE* fp = fopen("/sdcard/TTPOD_TTMediaPlayer.log", "a+"); \
					fprintf(fp, __VA_ARGS__); \
					fprintf(fp, "\n"); \
					fflush(fp); \
					fclose(fp);} 
#else
#include <android/log.h>
#define  LOG_TAG    "TTMediaPlayer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
#if defined (__COLLECT_LOG__)
#define	 LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,"TTCollectLog",__VA_ARGS__)
#else
#define	 LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,"LOG_TAG",__VA_ARGS__)
#endif
#endif
#else

#include <stdio.h>
#define  LOGI(...) printf(__VA_ARGS__); printf("\n")
#define  LOGD(...) printf(__VA_ARGS__); printf("\n")
#define  LOGE(...) printf(__VA_ARGS__); printf("\n")
#define  LOGW(...) printf(__VA_ARGS__); printf("\n")
#define  LOGV(...) printf(__VA_ARGS__); printf("\n")


#endif

#if defined(__TT_OS_IOS__)// IOS
#define __LOG_ALLOW__ 0
#define iOSVersion  [[UIDevice currentDevice].systemVersion floatValue]

#if __LOG_ALLOW__
#define NSLogDebug(...) NSLog(__VA_ARGS__)
#else
#    define NSLogDebug(...) /* */
#endif

#endif

#endif
