#ifndef __ST_OS_CONFIG_H__
#define __ST_OS_CONFIG_H__

#if defined (WIN32)
#define __ST_OS_WINDOWS__
#elif defined (__APPLE__)
#define __ST_OS_IOS__
#else
#define __ST_OS_ANDROID__
#endif

#if (defined __ST_OS_WINDOWS__)//WINDOWS
#define DLLIMPORT_C extern __declspec(dllimport)
#define DLLEXPORT_C __declspec(dllexport)
#define PLGINDLLEXPORT_C extern "C" __declspec(dllexport)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define STSetDbgFlag() _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF )
#define KMaxPathLength  (4096)

#elif (defined __ST_OS_IOS__)//IOS
#define DLLEXPORT_C
#define PLGINDLLEXPORT_C extern "C"
#define STSetDbgFlag()
#define KMaxPathLength  (4096)

#elif  (defined __ST_OS_ANDROID__)//ANDROID
#define PLGINDLLEXPORT_C extern "C"
#define DLLEXPORT_C
#define STSetDbgFlag()
#define KMaxPathLength  (4096)
#endif

#endif

//end of file
