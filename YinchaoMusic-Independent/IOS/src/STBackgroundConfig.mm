#include <Foundation/NSAutoreleasePool.h>
#include "STBackgroundConfig.h"
#include "STTypedef.h"
#include <AVFoundation/AVFoundation.h>
#include <AVFoundation/AVAudioSession.h>
#include "STLog.h"
#include "STBackgroundConfig.h"

STBool STBackgroundAudioQueueConfig::iBackgroundEnable = ESTFalse;
AudioQueueRef STBackgroundAudioQueueConfig::iAudioQueue = NULL;
AudioQueueBufferRef STBackgroundAudioQueueConfig::iAudioBuffer[KAudioQueueBufferNum] = {NULL, NULL, NULL};

STBackgroundAssetReaderConfig::STBackgroundAssetReaderConfig()
: iBackgroundEnable(ESTFalse)
, iAsset(NULL)
, iAssetReader(NULL)
, iAssetReaderOutput(NULL)
{
}

STBool STBackgroundAssetReaderConfig::IsEnable()
{
    return iBackgroundEnable;
}

STInt STBackgroundAssetReaderConfig::EnableBackground(const STChar* aPodUrl, STBool aEnable)
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
        
        iBackgroundEnable = ESTFalse;
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
            return  STKErrNotSupported;
        }
        
        NSError* Error = NULL;
        iAssetReader = (void*)[[AVAssetReader assetReaderWithAsset:((AVAsset*)iAsset) error:&Error] retain]; 
        
        AVAssetReaderAudioMixOutput * readerOutput = [AVAssetReaderAudioMixOutput assetReaderAudioMixOutputWithAudioTracks:pAudioArray audioSettings:nil];
        
        [(AVAssetReader*)iAssetReader addOutput:readerOutput];
        
        if (![(AVAssetReader*)iAssetReader startReading])
        {
            return STKErrNotReady;
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
        
        iBackgroundEnable = ESTTrue;
    }
    
    return STKErrNone;
}

STInt STBackgroundAudioQueueConfig::EnableBackground(STBool aEnable)
{
    if (iBackgroundEnable && !aEnable)
    {
        AudioQueueStop(iAudioQueue, ESTTrue);
        AudioQueueDispose(iAudioQueue, ESTTrue);
        iBackgroundEnable = ESTFalse;
    }
    else if (!iBackgroundEnable && aEnable)
    {
        iBackgroundEnable = (STKErrNone == StartAudioQueue());
    }
    
    return STKErrNone;
}

STInt STBackgroundAudioQueueConfig::StartAudioQueue()
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
        for (STInt i = 0; i < KAudioQueueBufferNum; i++)
        {
            Status = AudioQueueAllocateBuffer(iAudioQueue, KAudioQueueBufferSize, &iAudioBuffer[i]);
        }
    } 
    
    if (Status != noErr)
    {
        AudioQueueDispose(iAudioQueue, ESTTrue);
    }
    else
    {
        for (STInt i = 0; i < KAudioQueueBufferNum; i++)
        {
            STInt16* pDstBuffer = (STInt16*)iAudioBuffer[i]->mAudioData;
            memset(pDstBuffer, 0, KAudioQueueBufferSize);
            iAudioBuffer[i]->mAudioDataByteSize = KAudioQueueBufferSize;
            AudioQueueEnqueueBuffer(iAudioQueue, iAudioBuffer[i], 0, NULL);             
        }
       
        Status = AudioQueueStart(iAudioQueue, NULL);
        
        AudioQueuePause(iAudioQueue); 
    }
    
    if (Status != noErr) {
        AudioQueueDispose(iAudioQueue, ESTTrue);
    }
    
    return Status;
}

void STBackgroundAudioQueueConfig::AudioQueueCallback(void *aUserData, AudioQueueRef aAudioQueueRef, AudioQueueBufferRef aAudioQueueBufferRef)
{ 
    STInt16* pDstBuffer = (STInt16*)aAudioQueueBufferRef->mAudioData;
    memset(pDstBuffer, 0, KAudioQueueBufferSize);        
    aAudioQueueBufferRef->mAudioDataByteSize = KAudioQueueBufferSize;
    AudioQueueEnqueueBuffer(aAudioQueueRef, aAudioQueueBufferRef, 0, NULL);
}

