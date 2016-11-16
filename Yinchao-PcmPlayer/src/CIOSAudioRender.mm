//
//  CIOSAudioRender.cpp
//  AYClient-iOS
//
//  Created by Harris on 15/9/15.
//  Copyright (c) 2015年 LiRunhua. All rights reserved.
//
#ifdef __APPLE__

#include "CIOSAudioRender.h"
#import <Accelerate/Accelerate.h>
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/pixdesc.h"
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/Foundation.h>
#import <Accelerate/Accelerate.h>
#include "AyMovieGLViewProtocol.h"

CIOSAudioRender::CIOSAudioRender()
{
    m_pNativeWindow = NULL;
}

CIOSAudioRender::~CIOSAudioRender()
{
    UnInit();
}

int CIOSAudioRender::Init()
{
    return 0;
}

int CIOSAudioRender::UnInit()
{
    Stop();
    return 0;
}

int CIOSAudioRender::SetAudioFormat(AYMediaAudioFormat*pAudioFormat)
{
    CCriticalSection lock(&lock_);
    m_format = *pAudioFormat;
    if( m_pNativeWindow )
    {
        id<AyAudioProtocol> gl_view = (__bridge id<AyAudioProtocol>)m_pNativeWindow;
        [gl_view SetAudioFormat:&m_format];
        return 0;
    }

    
    //id<AyAudioManager> audioManager = [AyAudioManager audioManager];
    //[audioManager activateAudioSession:pAudioFormat->Channels with_samepleRate:pAudioFormat->SampleRate with_SampleBits:pAudioFormat->SampleBits];
    return -1;
}

void CIOSAudioRender::audioCallbackFillData(float * outData,uint32 numFrames,uint32 numChannels)
{
    /*
    if (_buffered) {
        memset(outData, 0, numFrames * numChannels * sizeof(float));
        return;
    }
    
    @autoreleasepool {
        
        while (numFrames > 0) {
            
            if (!_currentAudioFrame) {
                
                @synchronized(_audioFrames) {
                    
                    NSUInteger count = _audioFrames.count;
                    
                    if (count > 0) {
                        
                        KxAudioFrame *frame = _audioFrames[0];
                        

                        if (_decoder.validVideo) {
                            
                            const CGFloat delta = _moviePosition - frame.position;
                            
                            if (delta < -0.1) {
                                
                                memset(outData, 0, numFrames * numChannels * sizeof(float));

                                break; // silence and exit
                            }
                            
                            [_audioFrames removeObjectAtIndex:0];
                            
                            if (delta > 0.1 && count > 1) {
                                

                                continue;
                            }
                            
                        } else {
                            
                            [_audioFrames removeObjectAtIndex:0];
                            _moviePosition = frame.position;
                            _bufferedDuration -= frame.duration;
                        }
                        
                        _currentAudioFramePos = 0;
                        _currentAudioFrame = frame.samples;
                    }
                }
            }
            
            if (_currentAudioFrame) {
                
                const void *bytes = (Byte *)_currentAudioFrame.bytes + _currentAudioFramePos;
                const NSUInteger bytesLeft = (_currentAudioFrame.length - _currentAudioFramePos);
                const NSUInteger frameSizeOf = numChannels * sizeof(float);
                const NSUInteger bytesToCopy = MIN(numFrames * frameSizeOf, bytesLeft);
                const NSUInteger framesToCopy = bytesToCopy / frameSizeOf;
                
                memcpy(outData, bytes, bytesToCopy);
                numFrames -= framesToCopy;
                outData += framesToCopy * numChannels;
                
                if (bytesToCopy < bytesLeft)
                    _currentAudioFramePos += bytesToCopy;
                    else
                        _currentAudioFrame = nil;
                        
                        } else {
                            
                            memset(outData, 0, numFrames * numChannels * sizeof(float));
                            break;
                        }
        }
    }
     */
    memset(outData, 0, numFrames * numChannels * sizeof(float));
    CCriticalSection lock(&lock_);
    if(!queue_frames_.empty())
    {
        pair<int,boost::shared_array<uint8> > pair_current_frames = queue_frames_.front();
        queue_frames_.pop();
        
        memcpy(outData,pair_current_frames.second.get(),pair_current_frames.first);
        
    }

}

int CIOSAudioRender::SetNativeWindow(void*  pNativeWindow)
{
    m_pNativeWindow = pNativeWindow;
    return 0;
}

int CIOSAudioRender::Start()
{
    if( m_pNativeWindow )
    {
        id<AyAudioProtocol> gl_view = (__bridge id<AyAudioProtocol>)m_pNativeWindow;
        [gl_view PlayAudio];
        return 0;
    }
    /*
    CCriticalSection lock(&lock_);
    id<AyAudioManager> audioManager = [AyAudioManager audioManager];
    
    audioManager.outputBlock = ^(float *outData, UInt32 numFrames, UInt32 numChannels) {
        audioCallbackFillData(outData,numFrames,numChannels);
    };
    
    [audioManager play];*/
    
    return -1;
}

int CIOSAudioRender::Pause()
{
    if( m_pNativeWindow )
    {
        id<AyAudioProtocol> gl_view = (__bridge id<AyAudioProtocol>)m_pNativeWindow;
        [gl_view PauseAudio];
        return 0;
    }
    /*
    CCriticalSection lock(&lock_);
    id<AyAudioManager> audioManager = [AyAudioManager audioManager];

    [audioManager pause];
    audioManager.outputBlock = nil;*/
    return -1;
}

int CIOSAudioRender::Stop()
{
    if( m_pNativeWindow )
    {
        id<AyAudioProtocol> gl_view = (__bridge id<AyAudioProtocol>)m_pNativeWindow;
        [gl_view Stop];
        return 0;
    }
    return 0;
}

int CIOSAudioRender::Render(unsigned char*   pData, unsigned int  ulDataSize)
{
    if( m_pNativeWindow )
    {
        id<AyAudioProtocol> gl_view = (__bridge id<AyAudioProtocol>)m_pNativeWindow;
        return [gl_view RenderAudio:pData with_len:ulDataSize];
    }
    /*
    boost::shared_array<uint8> audio_data(new uint8[ulDataSize]);
    memcpy(audio_data.get(), pData, ulDataSize);
    
    CCriticalSection lock(&lock_);
    queue_frames_.push(pair<int,boost::shared_array<uint8> >(ulDataSize,audio_data));
    */
    return -1;
}

int CIOSAudioRender::Flush()
{
    return 0;
}

int CIOSAudioRender::GetPlayingTime(uint64*  pullTime)
{
    return 0;
}

int CIOSAudioRender::GetBufferTime(uint64*  pullTime)
{
    return 0;
}
int CIOSAudioRender::SetCallBack(void*   pCallbackFunc)
{
    return 0;
}

int CIOSAudioRender::GetRemainSize()
{
    return 0;
}

int CIOSAudioRender::SetVolume(int iVolume)
{
    return 0;
}

int CIOSAudioRender::GetVolume()
{
    return 0;
}

int CIOSAudioRender::GetLatency()
{
	return 0;
}
E_AUDIO_SAMPLE_FORMAT_TYPE   CIOSAudioRender::GetAudioSampleTypeFromFFmpeg(int  iFormat)
{
    CCriticalSection lock(&lock_);
    return m_format.eAudioFormatType;
}

#endif //__APPLE__