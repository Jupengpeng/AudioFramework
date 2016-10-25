#ifndef __TT_AUDIO_H__
#define __TT_AUDIO_H__

#if __cplusplus
extern "C" {
#endif

#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTMediadef.h"

// Channel Type
typedef enum {
	TT_AUDIO_CHANNEL_CENTER				= 1,	
	TT_AUDIO_CHANNEL_FRONT_LEFT			= 1<<1,	
	TT_AUDIO_CHANNEL_FRONT_RIGHT		= 1<<2,	
	TT_AUDIO_CHANNEL_SIDE_LEFT  		= 1<<3, 
	TT_AUDIO_CHANNEL_SIDE_RIGHT			= 1<<4, 
	TT_AUDIO_CHANNEL_BACK_LEFT			= 1<<5,	
	TT_AUDIO_CHANNEL_BACK_RIGHT			= 1<<6,	
	TT_AUDIO_CHANNEL_BACK_CENTER		= 1<<7,	
	TT_AUDIO_CHANNEL_LFE_BASS			= 1<<8,	
	TT_AUDIO_CHANNEL_ALL				= 0xffff,
	TT_AUDIO_CHANNEL_MAX				= TT_MAX_ENUM_VALUE
} TT_AUDIO_CODEC_CHANNELTYPE;

// Channel Mode 
typedef enum
{
	TT_AUDIO_CODEC_CHANNEL_STEREO		= 0x0000,		/* Stereo */
	TT_AUDIO_CODEC_CHANNEL_JOINTSTEREO	= 0x0001,		/* JS */
	TT_AUDIO_CODEC_CHANNEL_DUALMONO		= 0x0002,		/* Dual Mono */
	TT_AUDIO_CODEC_CHANNEL_MONO			= 0x0003,		/* Mono */
	TT_AUDIO_CODEC_CHANNEL_MULTICH		= 0x7F000001,	        /* multichannel */
	TT_AUDIO_CODEC_CHANNEL_MODE_MAX		= TT_MAX_ENUM_VALUE
}TT_AUDIO_CODEC_CHANNELMODE;

 
#define	TT_PID_AUDIO_BASE				0x02000000							/*!< The base param ID for AUDIO codec */
#define	TT_PID_AUDIO_FORMAT				(TT_PID_AUDIO_BASE | 0X0001)		/*!< The format data of audio in track */
#define	TT_PID_AUDIO_SAMPLEREATE		(TT_PID_AUDIO_BASE | 0X0002)		/*!< The samplerate of audio  */
#define	TT_PID_AUDIO_CHANNELS			(TT_PID_AUDIO_BASE | 0X0003)		/*!< The channel of audio */
#define	TT_PID_AUDIO_BITRATE			(TT_PID_AUDIO_BASE | 0X0004)		/*!< The bitrate of audio */
#define TT_PID_AUDIO_FLUSH              (TT_PID_AUDIO_BASE | 0X0005)     
#define TT_PID_AUDIO_HEAD_DATA	        (TT_PID_AUDIO_BASE | 0X0006)
#define TT_PID_AUDIO_DECODER_INFO	    (TT_PID_AUDIO_BASE | 0X0007)
#define TT_PID_AUDIO_SAMPLEREATE_MAX	(TT_PID_AUDIO_BASE | 0X0008)
#define TT_PID_AUDIO_DOWAVE				(TT_PID_AUDIO_BASE | 0X0009)
#define TT_PID_AUDIO_DOEFFECT			(TT_PID_AUDIO_BASE | 0X000A)
#define TT_PID_AUDIO_OFFSET				(TT_PID_AUDIO_BASE | 0X000B)


#define	TT_ERR_AUDIO_BASE				0x82000000
#define TTErrAudioUnChannel				TT_ERR_AUDIO_BASE | 0x0001
#define TTErrAudioUnSamplerate			TT_ERR_AUDIO_BASE | 0x0002
#define TTErrAudioUnFeature				TT_ERR_AUDIO_BASE | 0x0003

/* General audio codec function set */
typedef struct
{
	TTInt32 (*Open) (TTHandle * hHandle);
	TTInt32 (*SetInput) (TTHandle hHandle, TTBuffer *InBuffer);
	TTInt32 (*Process) (TTHandle hHandle, 
					   TTBuffer * OutBuffer, 
		               TTAudioFormat* pOutInfo);

	TTInt32 (*SetParam) (TTHandle hHandle, TTInt32 uParamID, TTPtr pData);
	TTInt32 (*GetParam) (TTHandle hHandle, TTInt32 uParamID, TTPtr pData);
	TTInt32 (*Close) (TTHandle hHandle);
} TTAudioCodecAPI;

typedef TTInt32 (*__GetAudioDECAPI)(TTAudioCodecAPI * pDecHandle);

#if __cplusplus
}  // extern "C"
#endif

#endif //__TT_AUDIO_H__
