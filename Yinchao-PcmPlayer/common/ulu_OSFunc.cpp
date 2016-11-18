#include "ulu_OSFunc.h"


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>
#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")
#include <stdio.h>
#elif defined _LINUX || defined (_IOS) || defined (_MAC_OS)
#include <stdio.h>
#include <unistd.h>
#include <time.h>      
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h> 
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <assert.h>
#endif

#if defined (_IOS) || defined (_MAC_OS)
#import <sys/sysctl.h>
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif
#import <mach/mach.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#ifdef _ULUNAMESPACE
namespace _ULUNAMESPACE {
#endif

#ifndef __APPLE__
void ulu_OS_Sleep (unsigned int nTime)
{
#ifdef WIN32
	Sleep (nTime);
#elif defined _LINUX
	usleep (1000 * nTime);
#elif defined _IOS || defined _MAC_OS
	usleep (1000 * nTime);
#endif // WIN32
}
#endif

void ulu_OS_SleepExitable(unsigned int nTime, bool * pbExit)
{
	unsigned int nStart = ulu_OS_GetSysTime();
	while(ulu_OS_GetSysTime() < nStart + nTime)
	{
		if(pbExit && true == *pbExit)
			return;

		ulu_OS_Sleep(5);
	}
}
#ifndef __APPLE__
unsigned int ulu_OS_GetSysTime (void)
{
	unsigned int	nTime = 0;

#ifdef WIN32
	nTime = ::timeGetTime();
#elif defined _LINUX
    timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);

    static timespec stv = {0, 0};
    if ((0 == stv.tv_sec) && (0 == stv.tv_nsec))
	{
		stv.tv_sec = tv.tv_sec;
		stv.tv_nsec = tv.tv_nsec;
	}
    
    nTime = (unsigned int)((tv.tv_sec - stv.tv_sec) * 1000 + (tv.tv_nsec - stv.tv_nsec) / 1000000);
#elif defined _IOS || defined _MAC_OS
    nTime = [[NSProcessInfo processInfo] systemUptime] * 1000;
#endif // WIN32

	return nTime;
}
#endif //__APPLE__
    
    
    
    
    
    
unsigned int ulu_OS_GetTickCount()
{

#ifdef __APPLE__
        static uint64_t tick_begin = mach_absolute_time();
        uint64_t endTime = mach_absolute_time();
        
        // Elapsed time in mach time units
        uint64_t elapsedTime = endTime - tick_begin;
        
        // The first time we get here, ask the system
        // how to convert mach time units to nanoseconds
        static double g_ticksToNanoseconds = 0.0;
        if (0.0 == g_ticksToNanoseconds)
        {
            mach_timebase_info_data_t timebase;
            
            // to be completely pedantic, check the return code of this next call.
            mach_timebase_info(&timebase);
            g_ticksToNanoseconds = (double)timebase.numer / timebase.denom;
        }
        double elapsedTimeInNanoseconds = (elapsedTime * g_ticksToNanoseconds) / NSEC_PER_SEC*1000;//1000000000.0;
        return ((int)elapsedTimeInNanoseconds);//&0xFFFF0;
#else
    
//#ifdef _WINDOWS
//    return GetTickCount();
//#else
//    struct timespec time_tmp={0,0};
//    clock_gettime(CLOCK_MONOTONIC,&time_tmp);
//    unsigned int ret = time_tmp.tv_sec;
//    ret *= 1000;
//    ret = ret + ((time_tmp.tv_nsec/1000000));//&0xFFFF0
//    return ret;
//#endif
	return GetTickCount();
    
#endif
}
    
    
    
    
    
    
    
    
    
    
#ifdef _ULUNAMESPACE
}
#endif //_ULUNAMESPACE

