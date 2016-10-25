/**
* File : TTAndroidVideoSink.cpp  
* Created on : 2014-11-5
* Author : yongping.lin
* Copyright : Copyright (c) 2014 Shuidushi Software Ltd. All rights reserved.
* Description : CTTIosVideoSink实现文件
*/

#include "TTOSALConfig.h"
// INCLUDES
#include "TTIosVideoSink.h"
#include "TTSleep.h"
#include "TTSysTime.h"
#include "TTLog.h"
#include "TTGLRenderES2.h"
#include "TTGLRenderES2_FTU.h"

extern TTInt gIos8Above;
//char* lpSurf1 = (char* )malloc(1024*4*1024);

CTTIosVideoSink::CTTIosVideoSink(CTTSrcDemux* aSrcMux, TTCBaseAudioSink* aAudioSink, TTDecoderType aDecoderType)
: TTCBaseVideoSink(aSrcMux, aAudioSink, aDecoderType)
, m_pRender(NULL)
{
	mCriView.Create();
	checkCPUFeature();
	checkHWEnable();
}

CTTIosVideoSink::~CTTIosVideoSink()
{
	closeVideoView();
	mCriView.Destroy();
}


TTInt CTTIosVideoSink::render()
{
	TTInt nErr = TTKErrNone;
	TTCAutoLock Lock(&mCriView);
    
    if(mSinkBuffer.Buffer[0] == NULL)
        return TTKErrArgument;
    /*int i=0, a=0;
    char* lpSurf = lpSurf1;
    if(lpSurf) {
        TTPBYTE pSrc = mSinkBuffer.Buffer[0];
        for (i = 0; i < mVideoFormat.Height; i++) {
            memcpy (lpSurf, pSrc, mVideoFormat.Width);
            
            pSrc += mSinkBuffer.Stride[0];
            lpSurf += mSinkBuffer.Stride[0]; a += mSinkBuffer.Stride[0];
        }
        
        pSrc = mSinkBuffer.Buffer[2];
        for (i = 0; i < mVideoFormat.Height / 2; i++) {
            memcpy(lpSurf, pSrc, mVideoFormat.Width / 2);
            
            pSrc += mSinkBuffer.Stride[2];
            lpSurf += mSinkBuffer.Stride[2]; a += mSinkBuffer.Stride[2];
        }
        
        pSrc = mSinkBuffer.Buffer[1];
        for (i = 0; i < mVideoFormat.Height / 2; i++) {
            memcpy(lpSurf, pSrc, mVideoFormat.Width / 2);
            
            pSrc += mSinkBuffer.Stride[1];
            lpSurf += mSinkBuffer.Stride[1]; a += mSinkBuffer.Stride[1];
        }
        static int ab =0;
        //if (ab == 0)
        ab++;
        if (ab == 1)
        {
            ab = 10;
            
            NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
            NSString *docDir = [paths objectAtIndex:0];
            const char* tmp = [docDir UTF8String];
            char* strpath = new char[strlen(tmp)+64];
            sprintf(strpath, "%s/1.yuv", tmp);
            FILE * F=  fopen(strpath,"wb+");
            fwrite(lpSurf1,1,a,F);
            fflush(F);
            fclose(F);
        }
    }*/
    nErr = renderYUV();
    
	return nErr;
}

TTInt CTTIosVideoSink::setView(void * pView)
{
	TTCAutoLock Lock(&mCriView);
	mView = pView;
    
    if(getPlayStatus() != EStatusStoped) {
        newVideoView();
    }

    return TTKErrNone;
}

TTInt CTTIosVideoSink::newVideoView()
{
	//closeVideoView ();

	TTCAutoLock Lock(&mCriView);
    LOGI("newVideoView::mView %d", mView);
    
    if(mView == NULL) {
        if(mVideoDecoder) {
            mVideoDecoder->stop();
        }
        return TTKErrNone;
    }

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    EAGLContext* pContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
    if (nil != pContext)
    {
        if (NULL == m_pRender) {
            if (gIos8Above > 0) {
                m_pRender = new TTGLRenderES2_FTU(pContext);
            }
            else
                m_pRender = new TTGLRenderES2(pContext);
            m_pRender->SetView((UIView*)mView);
            m_pRender->init();
            m_pRender->SetTexture(mVideoFormat.Width , mVideoFormat.Height);
        }
    }
    
    [pool release];

	if(mVideoDecoder ) {
        mHWDec = TT_VIDEODEC_SOFTWARE;
	}
    
    if(mVideoDecoder){
        if(getPlayStatus() == EStatusPlaying || getPlayStatus() == EStatusPaused)
            mVideoDecoder->start();
    }

	return TTKErrNone;
}

TTInt CTTIosVideoSink::closeVideoView()
{
	TTCAutoLock Lock(&mCriView);
    
    SAFE_DELETE(m_pRender);

	return TTKErrNone;
}


TTInt CTTIosVideoSink::renderYUV()
{
	if(mVideoFormat.Width == 0 || mVideoFormat.Height == 0 || mSinkBuffer.Buffer[0] == NULL || mSinkBuffer.Stride[0] == 0)	{
		return TTKErrNotReady;
	}
    
    TTCAutoLock Lock(&mCriView);
    
    if (m_pRender == NULL || mView == NULL) {
        return TTKErrNotReady;
    }

    return  m_pRender->RenderYUV(&mSinkBuffer);
}

void CTTIosVideoSink::checkCPUFeature()
{
    mCPUType = 7;
    mVideoDecoder->setParam(TT_PID_VIDEO_CPU_TYPE, &mCPUType);

	mCPUNum = 2;
	LOGI("CTTIosVideoSink::checkCPUFeature mCPUNum %d", mCPUNum);
	mVideoDecoder->setParam(TT_PID_VIDEO_THREAD_NUM, &mCPUNum);
}

TTInt CTTIosVideoSink::stop()
{
    TTCBaseVideoSink::stop();
    closeVideoView();
    return TTKErrNone;
}

