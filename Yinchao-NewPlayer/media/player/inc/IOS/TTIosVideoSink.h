/**
* File : TTAndroidVideoSink.h  
* Created on : 2014-11-5
* Author : yongping.lin
* Copyright : Copyright (c) 2014 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAndroidVideoSink定义文件
*/

#ifndef __TT_IOS_VIDEO_SINK_H__
#define __TT_IOS_VIDEO_SINK_H__

#include <stdio.h>
#include <unistd.h>

#include "TTBaseVideoSink.h"
#include "TTCritical.h"
//#include "TTGLRenderBase.h"

class TTGLRenderBase;
// CLASSES DECLEARATION
class CTTIosVideoSink : public TTCBaseVideoSink
{
public:
	CTTIosVideoSink(CTTSrcDemux* aSrcMux, TTCBaseAudioSink* aAudioSink, TTDecoderType aDecoderType);
	virtual ~CTTIosVideoSink();

public:
	virtual TTInt				render();

	virtual	TTInt				setView(void * pView);

	virtual TTInt				newVideoView();

	virtual TTInt				closeVideoView();

	virtual void				checkCPUFeature();
    
    virtual	TTInt				stop();
protected:
	virtual TTInt				renderYUV();

protected:

	RTTCritical					mCriView;
    
    TTGLRenderBase*             m_pRender;

};

#endif // __TT_WIN_AUDIO_SINK_H__