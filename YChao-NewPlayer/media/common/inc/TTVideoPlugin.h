/**
* File : TTVideoPlugin.h
* Created on : 2014-9-1
* Author : yongping.lin
* Copyright : Copyright (c) 2014 Shuidushi Software Ltd. All rights reserved.
* Description : TTVideoPlugin定义文件
*/


#ifndef __TT_VIDEOPLUGINMANAGER_H__
#define __TT_VIDEOPLUGINMANAGER_H__
#include "TTMediainfoDef.h"
#include "TTMediadef.h"
#include "TTCritical.h"
#include "TTVideo.h"
#include "ttHWVideoDec.h"

class CTTVideoPluginManager
{
public:
	CTTVideoPluginManager();
	~CTTVideoPluginManager();

	TTInt32							initPlugin(TTUint aFormat = 0, void* aInitParam = 0, TTInt aHwDecoder = 0);
	TTInt32							uninitPlugin();
	TTInt32							resetPlugin();
	TTInt32							setInput(TTBuffer *InBuffer);
	TTInt32							process(TTVideoBuffer* OutBuffer, TTVideoFormat* pOutInfo);
	TTInt32							setParam(TTInt32 uParamID, TTPtr pData);
	TTInt32							getParam(TTInt32 uParamID, TTPtr pData);

	static void						setPluginPath(const TTChar* aPath);
	
private:
	TTInt32							LoadLib ();

	TTVideoCodecAPI					mVideoCodecAPI;
	TTHandle						mHandle;
	TTUint							mFormat;
	TTInt							mCPUType;
	TTInt							mHwDecoder;
	void*							mParam;

	RTTCritical						mCritical;

	TTHandle						mLibHandle;
	static TTChar					mVideoPluginPath[256];
};


#endif
