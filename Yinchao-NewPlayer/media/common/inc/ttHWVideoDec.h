#ifndef __TTHWVIDEODEC_H__
#define __TTHWVIDEODEC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "TTOsalConfig.h"
#include "TTVideo.h"




#define TT_HWDECDEC_CALLBACK_ID_BASE               	0x04100000
#define TT_HWDECDEC_CALLBACK_PID_READBUFFER		 	(TT_HWDECDEC_CALLBACK_ID_BASE | 0x0001)  


DLLEXPORT_C TTInt32 ttGetH264DecAPI (TTVideoCodecAPI* pDecHandle);

DLLEXPORT_C TTInt32 ttGetMPEG4DecAPI (TTVideoCodecAPI* pDecHandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __ffmpegWrap_H__
