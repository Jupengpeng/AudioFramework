#ifndef __ULULOG_h__
#define __ULULOG_h__

using namespace std;

#define ULULOGW(fmt, ...)
#define ULULOGI(fmt, ...)
#define ULULOGE(fmt, ...)

#ifdef __APPLE__

#ifndef _IOS
#define _IOS
#endif //_IOS

#else

#ifdef ANDROID
#ifndef _LINUX
#define _LINUX
#endif //_LINUX

#ifndef _LINUX_ANDROID
#define _LINUX_ANDROID
#endif //_LINUX_ANDROID
#endif //ANDROID

#endif //__APPLE__

#ifdef _LINUX_ANDROID
#include <android/log.h>
#include <string.h>



#ifndef LOG_TAG
#define LOG_TAG "ANDROID_LAB_NDK"
#define LOGW(...) ((int)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGI(...) ((int)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((int)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif

#ifdef _DUMP_LOG
#undef  ULULOGE
#define ULULOGE(fmt, args...) \
{ \
    const char * p___Name = strrchr (__FILE__, '/'); \
    p___Name = ((p___Name == NULL) ? __FILE__ : (p___Name + 1)); \
    LOGE("ULUError: %s  %s  %d    " fmt "\n", p___Name, __FUNCTION__, __LINE__, ## args); \
}

#undef  ULULOGW
#define ULULOGW(fmt, args...) \
{ \
    const char * p___Name = strrchr (__FILE__, '/'); \
    p___Name = ((p___Name == NULL) ? __FILE__ : (p___Name + 1)); \
    LOGW("ULUWarning: %s  %s  %d    " fmt "\n", p___Name, __FUNCTION__, __LINE__, ## args); \
}

#undef  ULULOGI
#define ULULOGI(fmt, args...) \
{ \
    const char * p___Name = strrchr (__FILE__, '/'); \
    p___Name = ((p___Name == NULL) ? __FILE__ : (p___Name + 1)); \
    LOGI("ULUInfo: %s  %s  %d    " fmt "\n", p___Name, __FUNCTION__, __LINE__, ## args); \
}
#endif

#endif /* _LINUX_ANDROID */

#ifdef _WINDOWS
#include "stdio.h"
#include "time.h"
#include "string.h"
#include "windows.h"
#ifdef _DUMP_LOG
#undef  ULULOGE
#define ULULOGE(fmt, ...) \
{ \
	const char * p___Name = strrchr (__FILE__, '/'); \
	p___Name = ((p___Name == NULL) ? __FILE__ : (p___Name + 1)); \
	char szText[2048]; \
	TCHAR tszText[2048]; \
	sprintf(szText, "ULUError: [%u][%08X]  %s  %s  %d    " fmt "\n", ::GetTickCount(), GetCurrentThreadId(), (char*)p___Name, (char*)__FUNCTION__, __LINE__, __VA_ARGS__); \
	MultiByteToWideChar(CP_ACP, 0, szText, -1 , tszText, sizeof(tszText)); \
	OutputDebugString(tszText); \
}

#undef  ULULOGW
#define ULULOGW(fmt, ...) \
{ \
	const char * p___Name = strrchr (__FILE__, '/'); \
	p___Name = ((p___Name == NULL) ? __FILE__ : (p___Name + 1)); \
	char szText[2048]; \
	TCHAR tszText[2048]; \
	sprintf(szText, "ULUWarning: [%u][%08X]  %s  %s  %d    " fmt "\n", ::GetTickCount(), GetCurrentThreadId(), (char*)p___Name, (char*)__FUNCTION__, __LINE__, __VA_ARGS__); \
	MultiByteToWideChar(CP_ACP, 0, szText, -1 , tszText, sizeof(tszText)); \
	OutputDebugString(tszText); \
}



#undef  ULULOGI
#define ULULOGI(fmt, ...) \
{ \
	const char * p___Name = strrchr (__FILE__, '/'); \
	p___Name = ((p___Name == NULL) ? __FILE__ : (p___Name + 1)); \
	char szText[2048]; \
	TCHAR tszText[2048]; \
	sprintf(szText, "ULUInfo: [%u][%08X]  %s  %s  %d    " fmt "\n", ::GetTickCount(), GetCurrentThreadId(), (char*)p___Name, (char*)__FUNCTION__, __LINE__, __VA_ARGS__); \
	MultiByteToWideChar(CP_ACP, 0, szText, -1 , tszText, sizeof(tszText)); \
	OutputDebugString(tszText); \
}

#endif	// _DUMP_LOG
#endif	// _WINDOWS

#endif