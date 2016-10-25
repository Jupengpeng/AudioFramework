/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttTypeDef.h
*
*  File Description:Global Type define
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    14/May/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/
#ifndef __TT_TYPEDEF_T_H__
#define __TT_TYPEDEF_T_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TT_MAX_ENUM_VALUE	0X7FFFFFFF

typedef void TT_VOID;
typedef unsigned char TT_U8;
typedef unsigned char TT_BYTE;
typedef signed char TT_S8;
typedef char TT_CHAR;
typedef unsigned short TT_U16;

#if defined _WIN32
typedef unsigned short TT_WCHAR;
typedef unsigned short* TT_PWCHAR;
#elif defined LINUX
typedef unsigned char TT_WCHAR;
typedef unsigned char* TT_PWCHAR;
#endif

#ifdef _WIN32
#define TT_TCHAR		TCHAR
#define TT_PTCHAR		TCHAR*
#else
typedef char TCHAR, *PTCHAR;
#define TT_TCHAR		char
#define TT_PTCHAR		char*
#endif 

typedef signed short TT_S16;
typedef unsigned int TT_U32;
typedef signed int TT_S32;

#ifndef TT_SKIP64BIT
#ifdef _WIN32
typedef unsigned __int64  TT_U64;
typedef signed   __int64  TT_S64;
#else // WIN32
/** TT_U64 is a 64 bit unsigned quantity that is 64 bit word aligned */
typedef unsigned long long TT_U64;
/** TT_S64 is a 64 bit signed quantity that is 64 bit word aligned */
typedef signed long long TT_S64;
#endif // WIN32
#endif // TT_SKIP64BIT

typedef enum TT_BOOL {
    TT_FALSE = 0,
    TT_TRUE = !TT_FALSE,
    TT_BOOL_MAX = TT_MAX_ENUM_VALUE
} TT_BOOL;

typedef void* TT_PTR;
typedef const void* TT_CPTR;
typedef void* TT_HANDLE;
typedef char* TT_PCHAR;
typedef unsigned char* TT_PBYTE;

#ifdef _UNICODE
typedef unsigned short* TT_PTTCHAR;
typedef unsigned short TT_TTCHAR;
#else
typedef char* TT_PTTCHAR;
typedef char TT_TTCHAR;
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

typedef struct {
	TT_PBYTE	Buffer;		
	TT_U32		Length;		
	TT_S64		Time;		
	TT_PTR		UserData;   
} TT_CODECBUFFER;

/* General audio format info */
typedef struct
{
	TT_S32	SampleRate; 
	TT_S32	Channels;   
	TT_S32	SampleBits; 
} TT_AUDIO_FORMAT;

/* General audio output info */
typedef struct
{
	TT_AUDIO_FORMAT	Format;		
	TT_U32			InputUsed;	
	TT_U32			Resever;	
} TT_AUDIO_OUTPUTINFO;


#define TT_RETURN_OK				    0x00000000
#define TT_RETURN_BASE					0X82100000
#define TT_RETURN_FAILED				0x82100001
#define TT_RETURN_MEM_ERROR				0x82100002
#define TT_RETURN_NOT_IMPLEMENT			0x82100003
#define TT_RETURN_INVALID_ARG			0x82100004
#define TT_RETURN_INPUT_NO_ENOUGH		0x82100005
#define TT_RETURN_DROPPEDFRAME		    0x82100006
#define TT_RETURN_WRONG_STATUS			0x82100007
#define TT_RETURN_WRONG_PARAM_ID		0x82100008
#define TT_RETURN_LICENSE_ERROR			0x82100009
#define TT_RETURN_FORCE_STOP            0x8210000A

 
#define	TT_EQ_PID_BASE					0x42100000				
#define	TT_PID_HEADDATA			        (TT_EQ_PID_BASE | 0X0004)	
#define	TT_PID_FLUSH			        (TT_EQ_PID_BASE | 0X0005)	
#define	TT_PID_HEADINFO			        (TT_EQ_PID_BASE | 0X000B)	


#define	TT_PID_AUDIO_BASE				0x42000000							/*!< The base param ID for AUDIO codec */
#define	TT_PID_AUDIO_FORMAT				(TT_PID_AUDIO_BASE | 0X0001)		/*!< The format data of audio in track */
#define	TT_PID_AUDIO_SAMPLEREATE		(TT_PID_AUDIO_BASE | 0X0002)		/*!< The samplerate of audio  */
#define	TT_PID_AUDIO_CHANNELS			(TT_PID_AUDIO_BASE | 0X0003)		/*!< The channel of audio */
#define	TT_PID_AUDIO_BITRATE			(TT_PID_AUDIO_BASE | 0X0004)		/*!< The bitrate of audio */
#define TT_PID_COMMON_FLUSH             (TT_PID_AUDIO_BASE | 0X0005)            


#define	TT_ERR_AUDIO_BASE				0x82000000
#define TT_ERR_AUDIO_UNSCHANNEL			TT_ERR_AUDIO_BASE | 0x0001
#define TT_ERR_AUDIO_UNSSAMPLERATE		TT_ERR_AUDIO_BASE | 0x0002
#define TT_ERR_AUDIO_UNSFEATURE			TT_ERR_AUDIO_BASE | 0x0003


/* define the error ID */
#define TT_ERR_NONE						0x00000000
#define TT_ERR_OUTOF_MEMORY				0x80000002
#define TT_ERR_INVALID_ARG				0x80000004
#define TT_ERR_INPUT_BUFFER_SMALL		0x80000005
#define TT_ERR_OUTPUT_BUFFER_SMALL		0x80000006
#define TT_ERR_WRONG_PARAM_ID			0x80000008

/* General audio codec function set */
typedef struct
{
	TT_U32 (*Open) (TT_HANDLE * phEnc);
	TT_U32 (*SetInput) (TT_HANDLE hEnc, TT_CODECBUFFER *InBuffer);
	TT_U32 (*Process) (TT_HANDLE hEnc, 
					   TT_CODECBUFFER * OutBuffer, 
		               TT_AUDIO_OUTPUTINFO * pOutInfo);

	TT_U32 (*Set) (TT_HANDLE hEnc, TT_S32 uParamID, TT_PTR pData);
	TT_U32 (*Get) (TT_HANDLE hEnc, TT_S32 uParamID, TT_PTR pData);
	TT_U32 (*Close) (TT_HANDLE hEnc);
} TT_AUDIO_CODECAPI;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __TT_TYPEDEF_T_H__
