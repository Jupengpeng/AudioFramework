/**
* File : TTVideoPlugin.cpp  
* Created on : 2014-9-1
* Author : yongping.lin
* Copyright : Copyright (c) 2014 Shuidushi Software Ltd. All rights reserved.
* Description : TTVideoPlugin实现文件
*/

#include "TTVideoPlugin.h"
#include "TTMediainfoDef.h"
#include "TTDllLoader.h"
#include "TTOSALConfig.h"
#include "TTSysTime.h"
#include "TTLog.h"
#include "ttIosHwVideoDec.h"

extern TTInt gIos8Above;

TTChar CTTVideoPluginManager::mVideoPluginPath[256] = "";

CTTVideoPluginManager::CTTVideoPluginManager()
:mHandle(NULL)
,mLibHandle(NULL)
,mFormat(0)
,mCPUType(6)
,mParam(NULL)
,mHwDecoder(0)
{
	memset(&mVideoCodecAPI, 0, sizeof(mVideoCodecAPI));

	mCritical.Create();
}

CTTVideoPluginManager::~CTTVideoPluginManager()
{
	uninitPlugin();

	mCritical.Destroy();

}

TTInt32 CTTVideoPluginManager::initPlugin(TTUint aFormat, void* aInitParam, TTInt aHwDecoder)
{
	TTCAutoLock Lock(&mCritical);

	if((aFormat == 0 || aFormat == mFormat) && mHandle != NULL && mHwDecoder == aHwDecoder) {
		resetPlugin();
		if(aInitParam != NULL)
			mParam = aInitParam;

		setParam(TT_PID_VIDEO_DECODER_INFO, mParam);

		return TTKErrNone;
	}

	uninitPlugin();

	mHwDecoder = aHwDecoder;

	if(aFormat != 0)
		mFormat = aFormat;

	TTInt32 nErr = LoadLib();

	if(nErr != TTKErrNone)	
		return nErr;
	
	if(mVideoCodecAPI.Open == NULL)
		return TTKErrNotSupported;

	nErr = mVideoCodecAPI.Open(&mHandle);

	if(mHandle == NULL) 
		return TTKErrNotSupported;

	if(aInitParam != NULL)
		mParam = aInitParam;
	
	setParam(TT_PID_VIDEO_DECODER_INFO, mParam);
	
	return nErr;
}

TTInt32 CTTVideoPluginManager::uninitPlugin()
{
	TTCAutoLock Lock(&mCritical);
	if(mHandle == NULL || mVideoCodecAPI.Close == NULL)
		return TTKErrNotFound;

	TTInt nStop = 1;
	setParam(TT_PID_VIDEO_STOP, &nStop);

	mVideoCodecAPI.Close(mHandle);
	
	mHandle = NULL;

	memset(&mVideoCodecAPI, 0, sizeof(mVideoCodecAPI));
	
	return TTKErrNone;
}

TTInt32 CTTVideoPluginManager::resetPlugin()
{
	TTCAutoLock Lock(&mCritical);
	if(mHandle == NULL || mVideoCodecAPI.SetParam == NULL)
		return TTKErrNotFound;

	TTInt32 nFlush = 1;

	TTInt nErr = mVideoCodecAPI.SetParam(mHandle, TT_PID_VIDEO_FLUSH, &nFlush);

	return nErr;
}

TTInt32 CTTVideoPluginManager::setInput(TTBuffer *InBuffer)
{
	TTCAutoLock Lock(&mCritical);

	if(mHandle == NULL || mVideoCodecAPI.SetInput == NULL)
		return TTKErrNotFound;

	return mVideoCodecAPI.SetInput(mHandle, InBuffer);
}

TTInt32 CTTVideoPluginManager::process(TTVideoBuffer* OutBuffer, TTVideoFormat* pOutInfo)
{
	TTCAutoLock Lock(&mCritical);
	
	if(mHandle == NULL || mVideoCodecAPI.Process == NULL)
		return TTKErrNotFound;
	
	return mVideoCodecAPI.Process(mHandle, OutBuffer, pOutInfo);
}

TTInt32 CTTVideoPluginManager::setParam(TTInt32 uParamID, TTPtr pData)
{
	TTCAutoLock Lock(&mCritical);

	if(uParamID == TT_PID_VIDEO_CPU_TYPE) {
		mCPUType = *((TTInt*)pData); 
	}

	if(mHandle == NULL || mVideoCodecAPI.SetParam == NULL)
		return TTKErrNotFound;

	return mVideoCodecAPI.SetParam(mHandle, uParamID, pData);
}

TTInt32 CTTVideoPluginManager::getParam(TTInt32 uParamID, TTPtr pData)
{
	TTCAutoLock Lock(&mCritical);

	if(mHandle == NULL || mVideoCodecAPI.GetParam == NULL)
		return TTKErrNotFound;

	return mVideoCodecAPI.GetParam(mHandle, uParamID, pData);
}

void CTTVideoPluginManager::setPluginPath(const TTChar* aPath)
{
	/*if (aPath != NULL && strlen(aPath) > 0)
	{
		memset(mVideoPluginPath, 0, sizeof(TTChar)*256);
		strcpy(mVideoPluginPath, aPath);
	}*/
}

TTInt32 CTTVideoPluginManager::LoadLib ()
{
     __GetVideoDECAPI  pGetVideoDec = NULL;
    if (mFormat == TTVideoInfo::KTTMediaTypeVideoCodeH264) {
        if (gIos8Above == 0) {
            pGetVideoDec = ttGetH264DecAPI;
        }
        else{
            pGetVideoDec = ttHWGetH264DecAPI;
        }
		
	}

	if(pGetVideoDec == NULL) {
		LOGI("could not find video decoder api APIName");
		return TTKErrNotSupported;
	}

	return (*pGetVideoDec)(&mVideoCodecAPI);
}

//end of file