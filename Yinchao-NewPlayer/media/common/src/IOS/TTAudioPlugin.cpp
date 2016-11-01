/**
* File : TTAudioPlugin.cpp  
* Created on : 2014-9-1
* Author : yongping.lin
* Copyright : Copyright (c) 2014 Shuidushi Software Ltd. All rights reserved.
* Description : TTAudioPlugin实现文件
*/

#include "TTAudioPlugin.h"
#include "TTMediainfoDef.h"
#include "TTDllLoader.h"
#include "TTOSALConfig.h"
#include "TTSysTime.h"
#include "TTLog.h"

#include "ttAMRDec.h"
#include "ttAPEDec.h"
#include "ttALACDec.h"
#include "ttFLACDec.h"
#include "ttWMADec.h"
#include "ttIpodDec.h"
#include "ttAACDec.h"
//#if (defined __TT_OS_ANDROID__) && (defined CPU_ARM)
//#include <cpu-features.h>
//#endif

TTChar CTTAudioPluginManager::mPluginPath[256] = "";

CTTAudioPluginManager::CTTAudioPluginManager()
:mHandle(NULL)
,mLibHandle(NULL)
,mFormat(0)
{
	memset(&mAudioCodecAPI, 0, sizeof(mAudioCodecAPI));

	mCritical.Create();
	
#ifdef PERFORMANCE_PROFILE
	tAudioDecode = 0;
#endif
}

CTTAudioPluginManager::~CTTAudioPluginManager()
{
	uninitPlugin();

	mCritical.Destroy();

#ifdef PERFORMANCE_PROFILE
	LOGI("\tPERFORMANCE_PROFILE: AudioDecode = %lld\n", tAudioDecode);
#endif
}

TTInt32 CTTAudioPluginManager::initPlugin(TTUint aFormat, void* aInitParam, TTBool /*aDecodeOrEncode = ETTFalse*/)
{
	TTCAutoLock Lock(&mCritical);

	TTInt32 nErr = TTKErrNotSupported;

	uninitPlugin();

	mFormat = aFormat;

	nErr = LoadLib();

	if(nErr != TTKErrNone)	
		return nErr;
	
	if(mAudioCodecAPI.Open == NULL)
		return TTKErrNotSupported;

	nErr = mAudioCodecAPI.Open(&mHandle);

	if(mHandle == NULL || nErr != TTKErrNone)
		return TTKErrNotSupported;

	if(aInitParam != NULL)
		nErr = setParam(TT_PID_AUDIO_DECODER_INFO, aInitParam);

	return nErr;
}

TTInt32 CTTAudioPluginManager::uninitPlugin()
{
	TTCAutoLock Lock(&mCritical);
	if(mHandle == NULL || mAudioCodecAPI.Close == NULL)
		return TTKErrNotSupported;

	mAudioCodecAPI.Close(mHandle);
    
    mHandle = NULL;

	memset(&mAudioCodecAPI, 0, sizeof(mAudioCodecAPI));
	
	return TTKErrNone;
}


TTInt32 CTTAudioPluginManager::resetPlugin()
{
	TTCAutoLock Lock(&mCritical);
	if(mHandle == NULL || mAudioCodecAPI.SetParam == NULL)
		return TTKErrNotSupported;

	TTInt32 nFlush = 1;

	return mAudioCodecAPI.SetParam(mHandle, TT_PID_AUDIO_FLUSH, &nFlush);
}

TTInt32 CTTAudioPluginManager::setInput(TTBuffer *InBuffer)
{
	TTCAutoLock Lock(&mCritical);

	if(mHandle == NULL || mAudioCodecAPI.SetInput == NULL)
		return TTKErrNotSupported;

	return mAudioCodecAPI.SetInput(mHandle, InBuffer);
}

TTInt32 CTTAudioPluginManager::process(TTBuffer * OutBuffer, TTAudioFormat* pOutInfo)
{
	TTCAutoLock Lock(&mCritical);
	
	if(mHandle == NULL || mAudioCodecAPI.Process == NULL)
		return TTKErrNotSupported;
	
	return mAudioCodecAPI.Process(mHandle, OutBuffer, pOutInfo);
}

TTInt32 CTTAudioPluginManager::setParam(TTInt32 uParamID, TTPtr pData)
{
	TTCAutoLock Lock(&mCritical);

	if(mHandle == NULL || mAudioCodecAPI.SetParam == NULL)
		return TTKErrNotSupported;

	return mAudioCodecAPI.SetParam(mHandle, uParamID, pData);
}

TTInt32 CTTAudioPluginManager::getParam(TTInt32 uParamID, TTPtr pData)
{
	TTCAutoLock Lock(&mCritical);

	if(mHandle == NULL || mAudioCodecAPI.GetParam == NULL)
		return TTKErrNotSupported;

	return mAudioCodecAPI.GetParam(mHandle, uParamID, pData);
}

void CTTAudioPluginManager::setPluginPath(const TTChar* aPath)
{
	if (aPath != NULL && strlen(aPath) > 0)
	{
		memset(mPluginPath, 0, sizeof(TTChar)*256);
		strcpy(mPluginPath, aPath);
	}
}

TTInt32 CTTAudioPluginManager::LoadLib ()
{
    __GetAudioDECAPI  pGetAudioDec = NULL;
//    
	if (mFormat == TTAudioInfo::KTTMediaTypeAudioCodeMP3)
    {
        pGetAudioDec = ttGetMP3DecAPI;
    }
//	} else if (mFormat == TTAudioInfo::KTTMediaTypeAudioCodeAAC)
//    {
//		pGetAudioDec = ttGetAACDecAPI;
//	} else if (mFormat == TTAudioInfo::KTTMediaTypeAudioCodeAPE)
//    {
//		pGetAudioDec = ttGetAPEDecAPI;
//	} else if (mFormat==TTAudioInfo::KTTMediaTypeAudioCodeALAC)
//    {
//		pGetAudioDec = ttGetALACDecAPI;
//	} else if (mFormat==TTAudioInfo::KTTMediaTypeAudioCodeFLAC)
//    {
//		pGetAudioDec = ttGetFLACDecAPI;
//	//} else if (mFormat==TTAudioInfo::KTTMediaTypeAudioCodeDTS){
//	//	strcpy (APIName,"ttGetDTSDecAPI");
//	} else if (mFormat==TTAudioInfo::KTTMediaTypeAudioCodeWMA)
//    {
//		pGetAudioDec = ttGetWMADecAPI;
//	}  else if(mFormat == TTAudioInfo::KTTMediaTypeAudioCodeAMR)
//    {
//		pGetAudioDec = ttGetAMRDecAPI;
//	}
//    else if(mFormat==TTAudioInfo::KTTMediaTypeAudioCodeIPodLibrary)
//    {
//        pGetAudioDec = ttGetIPodDecAPI;
//    }
    
//    ttGetIPodDecAPI(&mAudioCodecAPI);

    if(pGetAudioDec == NULL)
    {
        //LOGI("could not find Audio decoder api APIName");
        return TTKErrNotSupported;
    }

	return (*pGetAudioDec)(&mAudioCodecAPI);
}

//end of file
