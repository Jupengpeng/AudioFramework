/**
* File        : TTTypeDef.h
* Created on  : 2011-2-11
* Author      : ran.zhao
* Copyright   : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
* Description : 定义类型 -- 声明文件
*/


#ifndef __TT_COMMANDID_H__
#define __TT_COMMANDID_H__
#include "TTOsalConfig.h"

enum TTCommandMsgId
{
	ECommandPlayerDestroyThread
	, ECommandPlayerOpen
	, ECommandPlayerStart
	, ECommandPlayerPause
	, ECommandPlayerResume
	, ECommandPlayerStop
	, ECommandPlayerSetVolume
	, ECommandPlayerGetVolume
    
	, ECommandPlayerSetPosition
	, ECommandPlayerGetPosition
    
	, ECommandPlayerGetFreq
	, ECommandPlayerGetWave
    
	, ECommandPlayerEnableAduioEffect
	, ECommandPlayerGetEQBandNum
    
	, ECommandPlayerSetSoundPriority
	, ECommandPlayerSetRepeat
	, ECommandPlayerRepeatCancel
	, ECommandPlayerSetVolumeRamp
    
	, ECommandPlayerSetPlayRange
	, ECommandConfigCacheFilePath
    
#ifdef __TT_OS_IOS__
	, ECommandPlayerPowerDown
#endif
    , ECommandDownLoadException
    , ECommandDownLoadPretchComplete
    , ECommandDownLoadPretchStart
    , ECommandDownLoadCacheComplete
    , ECommandDownLoadBufferDone
    
	////////////////////////////////////////////////////
};

#endif // __TT_COMMANDID_H__
