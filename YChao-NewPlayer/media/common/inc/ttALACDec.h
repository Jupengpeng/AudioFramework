
#ifndef __TTALACDEC_H__
#define __TTALACDEC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "TTOsalConfig.h"
#include "TTAudio.h"

/* ALAC Decoder APIs Interface */
DLLEXPORT_C TTInt32 ttGetALACDecAPI(TTAudioCodecAPI * pDecHandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __TTAPEDEC_H__
