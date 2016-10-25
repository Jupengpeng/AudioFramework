#ifndef __TT_VIDEO_H__
#define __TT_VIDEO_H__

#if __cplusplus
extern "C" {
#endif

#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTMediadef.h"
 
#define	TT_PID_VIDEO_BASE				0x04000000							
#define	TT_PID_VIDEO_FORMAT				(TT_PID_VIDEO_BASE | 0X0001)		
#define	TT_PID_VIDEO_BITRATE			(TT_PID_VIDEO_BASE | 0X0002)		
#define TT_PID_VIDEO_FLUSH              (TT_PID_VIDEO_BASE | 0X0003)     
#define TT_PID_VIDEO_HEAD_DATA	        (TT_PID_VIDEO_BASE | 0X0004)
#define TT_PID_VIDEO_DECODER_INFO	    (TT_PID_VIDEO_BASE | 0X0005)
#define TT_PID_VIDEO_PREVIEW			(TT_PID_VIDEO_BASE | 0X0006)
#define TT_PID_VIDEO_CPU_TYPE		    (TT_PID_VIDEO_BASE | 0X0007)
#define TT_PID_VIDEO_OUTPUT_NUM			(TT_PID_VIDEO_BASE | 0X0008)
#define TT_PID_VIDEO_THREAD_NUM			(TT_PID_VIDEO_BASE | 0X0009)
#define TT_PID_VIDEO_TIMERESET			(TT_PID_VIDEO_BASE | 0X000A)
#define TT_PID_VIDEO_SEEKOPTION			(TT_PID_VIDEO_BASE | 0X000B)
#define TT_PID_VIDEO_START				(TT_PID_VIDEO_BASE | 0X000C)
#define TT_PID_VIDEO_STOP				(TT_PID_VIDEO_BASE | 0X000D)
#define TT_PID_VIDEO_ENDEBLOCK			(TT_PID_VIDEO_BASE | 0X000E)
#define TT_PID_VIDEO_NATIVWINDOWS		(TT_PID_VIDEO_BASE | 0X000F)
#define TT_PID_VIDEO_RENDERBUFFER		(TT_PID_VIDEO_BASE | 0X0010)
#define TT_PID_VIDEO_FLUSHALL           (TT_PID_VIDEO_BASE | 0X0011)     
#define TT_PID_VIDEO_CALLFUNCTION		(TT_PID_VIDEO_BASE | 0X0012)


#define	TT_ERR_VIDEO_BASE				0x84000000
#define TTErrVideoUnFeature				TT_ERR_VIDEO_BASE | 0x0001

/* General video codec function set */
typedef struct
{
	TTInt32 (*Open) (TTHandle * hHandle);
	TTInt32 (*SetInput) (TTHandle hHandle, TTBuffer *InBuffer);
	TTInt32 (*Process) (TTHandle hHandle, 
					   TTVideoBuffer* OutBuffer, 
		               TTVideoFormat* pOutInfo);

	TTInt32 (*SetParam) (TTHandle hHandle, TTInt32 uParamID, TTPtr pData);
	TTInt32 (*GetParam) (TTHandle hHandle, TTInt32 uParamID, TTPtr pData);
	TTInt32 (*Close) (TTHandle hHandle);
} TTVideoCodecAPI;

typedef TTInt32 (*__GetVideoDECAPI)(TTVideoCodecAPI * pDecHandle);

#if __cplusplus
}  // extern "C"
#endif

#endif //__TT_AUDIO_H__
