//
//  CIOSAudioRender.h
//  AYClient-iOS
//
//  Created by Harris on 15/9/15.
//  Copyright (c) 2015年 LiRunhua. All rights reserved.
//

#ifndef __AYClient_iOS__CIOSAudioRender__
#define __AYClient_iOS__CIOSAudioRender__

#ifdef __APPLE__

#include "IAudioRender.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/pixdesc.h"

#include "base/CriticalSectionMgr.h"
#include <queue>
using namespace std;

#include <boost/shared_array.hpp>

class CIOSAudioRender:public IAudioRender
{
public:
    CIOSAudioRender();
    virtual ~CIOSAudioRender();
    virtual int Init();
    virtual int UnInit();
    virtual int SetAudioFormat(AYMediaAudioFormat*pAudioFormat);
    virtual int SetNativeWindow(void*  pNativeWindow);
    virtual int Start();
    virtual int Pause();
    virtual int Stop();
    virtual int Render(unsigned char*   pData, unsigned int  ulDataSize);
    virtual int Flush();
    virtual int GetPlayingTime(uint64*  pullTime);
   	virtual int GetBufferTime(uint64*  pullTime);
    virtual int SetCallBack(void*   pCallbackFunc);
    virtual int GetRemainSize();
    virtual int SetVolume(int iVolume);
    virtual int GetVolume();
	virtual int GetLatency();
    virtual E_AUDIO_SAMPLE_FORMAT_TYPE   GetAudioSampleTypeFromFFmpeg(int  iFormat);
    
public:
    AYMediaAudioFormat m_format;
    void * m_pNativeWindow;
    CCriticalSectionMgr lock_;
    void audioCallbackFillData(float * outData,uint32 numFrames,uint32 numChannels);
    
    queue< pair<uint32,boost::shared_array<uint8> > > queue_frames_;
    
};


#endif

#endif /* defined(__AYClient_iOS__CIOSAudioRender__) */
