#ifndef __ULU_OSFUNC_H__
#define __ULU_OSFUNC_H__

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

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */
#endif

void			ulu_OS_Sleep (unsigned int nTime);
// this function is exit-able but not accurate, please use it carefully
void			ulu_OS_SleepExitable (unsigned int nTime, bool * pbExit);
unsigned int	ulu_OS_GetSysTime (void);
unsigned int    ulu_OS_GetTickCount();
        
#ifdef __APPLE__
const char *    uluGetLogPath();
#endif
        
#ifdef _ULUNAMESPACE
}
#else
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif //__ULU_OSFUNC_H__