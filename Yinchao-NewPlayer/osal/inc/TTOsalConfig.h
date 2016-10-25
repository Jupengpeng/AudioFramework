#ifndef __TT_OSAL_CONFIG_H__
#define __TT_OSAL_CONFIG_H__

//#define PERFORMANCE_PROFILE

#if defined (WIN32)
#define __TT_OS_WINDOWS__
#elif defined (__SYMBIAN32__)
#define  __TT_OS_SYMBIAN__
#elif defined (__APPLE__)
#define __TT_OS_IOS__
#else
#define __TT_OS_ANDROID__
#endif

#if (defined __TT_OS_WINDOWS__)//WINDOWS
#define DLLIMPORT_C extern __declspec(dllimport)
#define DLLEXPORT_C __declspec(dllexport)
#define PLGINDLLEXPORT_C extern "C" __declspec(dllexport)
#define INLINE_C 
#define IBSS_ATTR
#define ICONST_ATTR
#define __attribute__(x)

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define TTSetDbgFlag() _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF )
#define TTDecodeNotifyInit()
#define TTDecodeNotifyUninit()
#define ENODATA 61 

#define HARDWARE_MAXSAMPLERATE		44100

static const int KDecodePCMBufferSize = 960 * 1024;//PCM数据缓存大小对应Transfer后端
static const int KSinkBufferSize = 80 * 1024;//PCM每个buffer数据缓存大小对应Transfer每次解码的DstBuffer大小
static const int KMinSnkBufferNum = 6;//Sink端缓存的Buffer个数必须小于等于KDecodePCMBufferSize/KSinkBufferSize
static const int KHwAudioBuffer = 80 * 1024;//写入音频中的每个Buffer的大小
static const int KMaxEmptyBufferNum = 4;//用于可视化，缓存已经写入音频的Buffer的个数 必须小于KMinSnkBufferNum
static const char* KSystemLibPath = "C:\\WINDOWS\\system32";

static const char* KCodecWildCard = ".dll";
static const char* KEffectWildCard = ".dll";

#elif (defined __TT_OS_SYMBIAN__)//SYMBIAN
#define DLLIMPORT_C 
#define DLLEXPORT_C __declspec(dllexport)
#define PLGINDLLEXPORT_C extern "C" __declspec(dllexport)
#define INLINE_C inline
#define IBSS_ATTR
#define ICONST_ATTR

#define HARDWARE_MAXSAMPLERATE		44100

static const char* KSystemLibPath = "/system/lib/";

#define TTSetDbgFlag()
#define TTDecodeNotifyInit()
#define TTDecodeNotifyUninit()

static const int KDecodePCMBufferSize = 320 * 1024;//PCM数据缓存大小对应Transfer后端
static const int KSinkBufferSize = 40 * 1024;//PCM每个buffer数据缓存大小对应Transfer每次解码的DstBuffer大小
static const int KMinSnkBufferNum = 8;//Sink端缓存的Buffer个数必须小于等于KDecodePCMBufferSize/KSinkBufferSize
static const int KHwAudioBuffer = 40 * 1024;//写入音频中的每个Buffer的大小
static const int KMaxEmptyBufferNum = 4;//用于可视化，缓存已经写入音频的Buffer的个数 必须小于KMinSnkBufferNum

static const char* KCodecWildCard = ".dll";
static const char* KEffectWildCard = ".dll";


#elif (defined __TT_OS_ANDROID__)//ANDROID
#define DLLIMPORT_C 
#define DLLEXPORT_C __attribute__ ((visibility("default")))
#define PLGINDLLEXPORT_C extern "C" __attribute__ ((visibility("default")))
#define INLINE_C inline
#define IBSS_ATTR
#define ICONST_ATTR
#define __COLLECT_LOG__

#define TTSetDbgFlag()
extern  void NotifyDecThreadAttach();
extern  void NotifyDecThreadDetach();
#define TTDecodeNotifyInit() NotifyDecThreadAttach()
#define TTDecodeNotifyUninit() NotifyDecThreadDetach()

#define HARDWARE_MAXSAMPLERATE		44100

static const int KDecodePCMBufferSize = 960 * 1024;//PCM数据缓存大小对应Transfer后端
static const int KSinkBufferSize = 80 * 1024;//PCM每个buffer数据缓存大小对应Transfer每次解码的DstBuffer大小
static const int KMinSnkBufferNum = 8;//Sink端缓存的Buffer个数必须小于等于KDecodePCMBufferSize/KSinkBufferSize
static const int KHwAudioBuffer = 80 * 1024;//写入音频中的每个Buffer的大小
static const int KMaxEmptyBufferNum = 8;//用于可视化，缓存已经写入音频的Buffer的个数
static const char* KSystemLibPath = "/system/lib/";

static const char* KCodecWildCard = ".so";
static const char* KEffectWildCard = ".so";

//#define __BufferingTest__
#elif (defined __TT_OS_IOS__)//IOS
#define INLINE_C 
#define IBSS_ATTR
#define ICONST_ATTR
#define PLGINDLLEXPORT_C extern "C"
#define DLLEXPORT_C

#define TTSetDbgFlag()
#define TTDecodeNotifyInit()
#define TTDecodeNotifyUninit()

#define HARDWARE_MAXSAMPLERATE		44100

static const int KDecodePCMBufferSize = 640 * 1024;//PCM数据缓存大小对应Transfer后端
static const int KSinkBufferSize = 80 * 1024;//PCM每个buffer数据缓存大小对应Transfer每次解码的DstBuffer大小
static const int KMinSnkBufferNum = 2;//Sink端缓存的Buffer个数必须小于等于KDecodePCMBufferSize/KSinkBufferSize
static const int KHwAudioBuffer = 80 * 1024;//写入音频中的每个Buffer的大小
static const int KMaxEmptyBufferNum = 1;//用于可视化，缓存已经写入音频的Buffer的个数 必须小于KMinSnkBufferNum
#endif

static const int KDemuxAudioBufferSize = 64 * 1024;
#endif

//end of file
