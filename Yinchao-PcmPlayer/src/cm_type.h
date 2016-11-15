/*******************************************************************************
    File: CThread.h
    Contains: unified platform thread define header file
    Written by: Shen qichao
    Change History (most recent first):
    2015-01-15 SQC Create file
*******************************************************************************/

#ifndef __UNIFIED_TYPE_H__
#define __UNIFIED_TYPE_H__

#ifndef CM_MAX_ENUM_VLAUE
#define CM_MAX_ENUM_VLAUE 0x7fffffff
#endif

#ifndef uint8
typedef unsigned char uint8;
#endif


/** int8_t is an 8 bit signed quantity that is byte aligned */
#ifndef int8
typedef signed char int8;
#endif

/** uint16_t is a 16 bit unsigned quantity that is 16 bit word aligned */
#ifndef uint16
typedef unsigned short uint16;
#endif

/** int16_t is a 16 bit signed quantity that is 16 bit word aligned */
#ifndef int16
typedef signed short int16;
#endif

/** uint32_t is a 32 bit unsigned quantity that is 32 bit word aligned */
#ifndef uint32
typedef unsigned int uint32;
#endif
	
/** int32_t is a 32 bit signed quantity that is 32 bit word aligned */
#ifndef int32
typedef signed int int32;
#endif


#ifndef int64
#ifdef _WINDOWS
/** int64_t is a 64 bit signed quantity that is 64 bit word aligned */
typedef signed   __int64  int64;
#else // _WINDOWS
/** int64_t is a 64 bit signed quantity that is 64 bit word aligned */
typedef signed long long int64;
#endif // _WINDOWS
#endif


#ifndef uint64
#ifdef _WINDOWS
/** uint64_t is a 64 bit unsigned quantity that is 64 bit word aligned */
typedef unsigned  __int64  uint64;
#else // _WINDOWS
/** uint64_t is a 64 bit unsigned quantity that is 64 bit word aligned */
typedef unsigned long long uint64;
#endif // _WINDOWS
#endif


#endif

