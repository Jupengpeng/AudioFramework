/**
 * File : TTBackgroundConfig.mm
 * Created on : 2011-9-7
 * Author : hu.cao
 * Copyright : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
 * Description : TTBackgroundConfig 
 */

#include <Foundation/NSAutoreleasePool.h>
#include "TTBackgroundConfig.h"
#include "TTTypedef.h"
#include <AVFoundation/AVFoundation.h>
#include <AVFoundation/AVAudioSession.h>
#include "TTLog.h"

extern TTBool gIsIOS4X;
TTBool TTBackgroundAudioQueueConfig::iBackgroundEnable = ETTFalse;
AudioQueueRef TTBackgroundAudioQueueConfig::iAudioQueue = NULL;
AudioQueueBufferRef TTBackgroundAudioQueueConfig::iAudioBuffer[KAudioQueueBufferNum] = {NULL, NULL, NULL};

TTBackgroundAssetReaderConfig::TTBackgroundAssetReaderConfig()
: iBackgroundEnable(ETTFalse)
, iAsset(NULL)
, iAssetReader(NULL)
, iAssetReaderOutput(NULL)
{
}

TTBool TTBackgroundAssetReaderConfig::IsEnable()
{
    return iBackgroundEnable;
}

TTInt TTBackgroundAssetReaderConfig::EnableBackground(const TTChar* aPodUrl, TTBool aEnable)
{
    if (iBackgroundEnable && !aEnable) 
    {
        //Disable 
     
        [(AVAssetReader*)iAssetReader cancelReading];
        [(AVAssetReaderAudioMixOutput*)iAssetReaderOutput release];
        iAssetReaderOutput = NULL;
        

        [(AVAssetReader*)iAssetReader release];
        iAssetReader = NULL;
        
        [(AVAsset*)iAsset release];
        iAsset = NULL;
        
        iBackgroundEnable = ETTFalse;
    }
    else if (!iBackgroundEnable && aEnable)    
    {
        //Enable        
    
        NSString* pStr = [[NSString alloc] initWithUTF8String:aPodUrl];    
        NSURL* pNewUrl = [[NSURL alloc] initWithString:pStr];
        
        iAsset = (void*)([[AVURLAsset URLAssetWithURL:pNewUrl options:nil] retain]);  
        
        [pStr release];
        [pNewUrl release];
        
        NSArray* pAudioArray = [((AVAsset*)iAsset) tracksWithMediaType:AVMediaTypeAudio];
        
        if ([pAudioArray count] <= 0)
        {
            [((AVAsset*)iAsset) release];
            iAsset = NULL;
            NSLogDebug(@"TTBackgroundAssetReaderConfig AssetReader count == 0");
            return  TTKErrNotSupported;
        }
        
        NSError* Error = NULL;
        iAssetReader = (void*)[[AVAssetReader assetReaderWithAsset:((AVAsset*)iAsset) error:&Error] retain]; 
        
        AVAssetReaderAudioMixOutput * readerOutput = [[AVAssetReaderAudioMixOutput assetReaderAudioMixOutputWithAudioTracks:pAudioArray audioSettings:nil] retain];
        
        [(AVAssetReader*)iAssetReader addOutput:readerOutput];
        
        if (![(AVAssetReader*)iAssetReader startReading])
        {
            NSLogDebug(@"TTBackgroundAssetReaderConfig AssetReader TTKErrNotReady");
            return TTKErrNotReady;
        }
        
        CMSampleBufferRef pSample = NULL;
        
        if(((AVAssetReader*)iAssetReader).status == AVAssetReaderStatusReading)
        {   
            pSample = [readerOutput copyNextSampleBuffer];
            
            if(pSample != NULL)
            {
                CFRelease(pSample);
            }
        }
        
        iBackgroundEnable = ETTTrue;
    }
    
    return TTKErrNone;
}

TTInt TTBackgroundAudioQueueConfig::EnableBackground(TTBool aEnable)
{
    TTInt nErr = TTKErrNone;
    if (iBackgroundEnable && !aEnable)
    {
        //Disable
        AudioQueueStop(iAudioQueue, ETTTrue);
        AudioQueueDispose(iAudioQueue, ETTTrue);
        iBackgroundEnable = ETTFalse;
    }
    else if (!iBackgroundEnable && aEnable)
    {
        //Enable
        nErr = StartAudioQueue();
        if (nErr == TTKErrNone) {
            iBackgroundEnable = ETTTrue;
        }
    }
    
    return TTKErrNone;
}

TTInt TTBackgroundAudioQueueConfig::StartAudioQueue()
{
    AudioStreamBasicDescription tAudioStreamDataFormat;
    tAudioStreamDataFormat.mSampleRate = 8000;
    tAudioStreamDataFormat.mFormatID = kAudioFormatLinearPCM;  
    tAudioStreamDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;  
    tAudioStreamDataFormat.mBytesPerPacket = 4;  
    tAudioStreamDataFormat.mFramesPerPacket = 1;  
    tAudioStreamDataFormat.mBytesPerFrame = 4;  
    tAudioStreamDataFormat.mChannelsPerFrame = 1;  
    tAudioStreamDataFormat.mBitsPerChannel = 16;  
    
    OSStatus Status = AudioQueueNewOutput(&tAudioStreamDataFormat, AudioQueueCallback, NULL, NULL, kCFRunLoopCommonModes, 0, &iAudioQueue);
    
    if (Status == noErr)
    {
        for (TTInt i = 0; i < KAudioQueueBufferNum; i++)
        {
            Status = AudioQueueAllocateBuffer(iAudioQueue, KAudioQueueBufferSize, &iAudioBuffer[i]);
        }
    } 
    
    if (Status != noErr)
    {
        AudioQueueDispose(iAudioQueue, ETTTrue);
    }
    else
    {
        for (TTInt i = 0; i < KAudioQueueBufferNum; i++)
        {
            TTInt16* pDstBuffer = (TTInt16*)iAudioBuffer[i]->mAudioData;
            memset(pDstBuffer, 0, KAudioQueueBufferSize);
            iAudioBuffer[i]->mAudioDataByteSize = KAudioQueueBufferSize;
            AudioQueueEnqueueBuffer(iAudioQueue, iAudioBuffer[i], 0, NULL);             
        }
       
        Status = AudioQueueStart(iAudioQueue, NULL);
        
        AudioQueuePause(iAudioQueue); 
    }
    
    if (Status != noErr) {
        AudioQueueDispose(iAudioQueue, ETTTrue);
    }
    
    return Status;
}

void TTBackgroundAudioQueueConfig::AudioQueueCallback(void *aUserData, AudioQueueRef aAudioQueueRef, AudioQueueBufferRef aAudioQueueBufferRef)
{ 
    TTInt16* pDstBuffer = (TTInt16*)aAudioQueueBufferRef->mAudioData;
    memset(pDstBuffer, 0, KAudioQueueBufferSize);        
    aAudioQueueBufferRef->mAudioDataByteSize = KAudioQueueBufferSize;
    AudioQueueEnqueueBuffer(aAudioQueueRef, aAudioQueueBufferRef, 0, NULL);
}

