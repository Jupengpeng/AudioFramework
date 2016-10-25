/**
* File : ttClConv.h
* Created on : 2014-11-1
* Author : yongping.lin
* Copyright : Copyright (c) 2014 Shuidushi Software Ltd. All rights reserved.
* Description : ttClConv定义文件
*/

#ifndef __TTCLCONV_H__
#define __TTCLCONV_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "TTOsalConfig.h"
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "TTMediadef.h"

DLLEXPORT_C void colorConvAlpha(TTUint alpha);

DLLEXPORT_C TTInt colorConvYUV2RGB(TTVideoBuffer* aSrc, TTVideoBuffer *sDst, TTInt aWidth, TTInt aHeight);

typedef void (*_colorConvAlpha)(TTUint alpha);

typedef TTInt (*_colorConvYUV2RGB)(TTVideoBuffer* aSrc, TTVideoBuffer *sDst, TTInt aWidth, TTInt aHeight);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __TTCLCONV_H__
