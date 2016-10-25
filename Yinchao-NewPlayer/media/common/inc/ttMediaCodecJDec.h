#ifndef __TTMEDIACODECJAVA_H__
#define __TTMEDIACODECJAVA_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "TTOsalConfig.h"
#include "TTVideo.h"

DLLEXPORT_C TTInt32 ttGetH264DecAPI (TTVideoCodecAPI* pDecHandle);

DLLEXPORT_C TTInt32 ttGetMPEG4DecAPI (TTVideoCodecAPI* pDecHandle);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __TTMEDIACODECJAVA_H__
