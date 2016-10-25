
#ifndef __TTAPEDEC_H__
#define __TTAPEDEC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "TTOsalConfig.h"
#include "TTAudio.h"

/* APE Decoder APIs Interface */
DLLEXPORT_C TTInt32 ttGetAPEDecAPI(TTAudioCodecAPI * pDecHandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __TTAPEDEC_H__
