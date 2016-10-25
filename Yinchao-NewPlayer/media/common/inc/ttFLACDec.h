
#ifndef __TTFLACDEC_H__
#define __TTFLACDEC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "TTOsalConfig.h"
#include "TTAudio.h"

/* FLAC Decoder APIs Interface */
DLLEXPORT_C TTInt32 ttGetFLACDecAPI(TTAudioCodecAPI * pDecHandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __TTAPEDEC_H__
