#include <stdio.h>
#include <string.h>
#include "STMacrodef.h"
#include "STTypedef.h"
#include "STIPodLibraryPlugin.h"
#include "STOSConfig.h"
#include "STSampleBuffer.h"
#import <AVFoundation/AVFoundation.h>
#include <AVFoundation/AVAudioSession.h>

STBool STIPodLibraryPlugin::IsSourceValid(const STChar* aUrl)
{   
    return ESTTrue;
}

STIPodLibraryPlugin::STIPodLibraryPlugin()
: iAsset(NULL)
, iAssetReader(NULL)
, iAssetReaderOutput(NULL)
, iCurSampleBufferRef(NULL)
, iTotalReadSize(0)
, iAudioArray(NULL)
, iCurSampleBufferDataOffset(0)
, iCurSampleBufferTotalLen(0)
, iCurSampleBufferDataPtr(NULL)
{
    
}

STIPodLibraryPlugin::~STIPodLibraryPlugin()
{
}

STInt STIPodLibraryPlugin::InitPlugin(const STChar* aUrl)
{
    ReleaseAsset();
    NSString* pStr = [[NSString alloc] initWithUTF8String:aUrl];    
    NSURL* pNewUrl = [[NSURL alloc] initWithString:pStr];
    
    iAsset = (void*)([[AVURLAsset URLAssetWithURL:pNewUrl options:nil] retain]);  
    
    [pStr release];
    [pNewUrl release];
    
    iAudioArray = (void*)[[((AVAsset*)iAsset) tracksWithMediaType:AVMediaTypeAudio] retain];
    
    if ([(NSArray*)iAudioArray count] > 0)
    {
        AVAssetTrack* pSongTrack = [(NSArray*)iAudioArray objectAtIndex:0];    
        
        NSArray* formatDesc = pSongTrack.formatDescriptions;
        for(unsigned int i = 0; i < [formatDesc count]; ++i)
        {
            CMAudioFormatDescriptionRef pItem = (CMAudioFormatDescriptionRef)[formatDesc objectAtIndex:i];
            const AudioStreamBasicDescription* pDesc = CMAudioFormatDescriptionGetStreamBasicDescription (pItem);
            
            if (pDesc != NULL)
            {
                iMediaInfo.iSampleRate = pDesc->mSampleRate;
                iMediaInfo.iChannels = pDesc->mChannelsPerFrame;
            }
        }       
    }
    
    if ((iMediaInfo.iSampleRate <= 0) || ((iMediaInfo.iChannels != 1) && (iMediaInfo.iChannels != 2)))
    {
        [(NSArray*)iAudioArray release];
        [((AVAsset*)iAsset) release];
        iAudioArray = NULL;
        iAsset = NULL;
        return STKErrNotSupported;
    }
    
    
    iDuration = (STUint)(CMTimeGetSeconds(((AVAsset*)iAsset).duration) * 1000);
    
    return STKErrNone;
}

void STIPodLibraryPlugin::UnInitPlugin()
{
    STBasePlugin::UnInitPlugin();
    ReleaseReader();
    ReleaseAsset();
    iTotalReadSize = 0;
}

void STIPodLibraryPlugin::ReleaseAsset()
{
    if (iAudioArray != NULL)
    {
        [(NSArray*)iAudioArray release];
        iAudioArray = NULL;
    }
    
    if (iAsset != NULL)
    {
        [(AVAsset*)iAsset release];
        iAsset = NULL;
    }
    
    iMediaInfo.iSampleRate = 0;
    iMediaInfo.iChannels = 0;
}

void STIPodLibraryPlugin::ResetPlugin()
{
    ReleaseReader();
}

void STIPodLibraryPlugin::ReleaseReader()
{
    PreSampleRefRelease();
    if (iAssetReader != NULL)
    {
        [(AVAssetReader*)iAssetReader cancelReading];
    
        [(AVAssetReaderAudioMixOutput*)iAssetReaderOutput release];
        iAssetReaderOutput = NULL;
   
        [(AVAssetReader*)iAssetReader release];
        iAssetReader = NULL;
    }
    
    iCurSampleBufferDataPtr = NULL;
    iCurSampleBufferDataOffset = 0;
    iCurSampleBufferTotalLen = 0;
    
    iReadStatus = ESTReadStatusNotReady;
}

STInt STIPodLibraryPlugin::StartReading()
{
    return StartReadingAt(0);
}

STInt STIPodLibraryPlugin::Read(STSampleBuffer* aBuffer)
{        
    STASSERT(aBuffer->StartPos() == 0);
    STInt nErr = STKErrNone;
    
    while (ESTTrue)
    {
        if (STKErrNone == (nErr = FillSampleBuffer(aBuffer))) 
        {
            break;
        }
        
        if (STKErrNone != (nErr = ReadNextSampleBuffer()))
        {
            memset(aBuffer->Ptr() + aBuffer->StartPos(), 0, aBuffer->ValidSize());
            aBuffer->SetPosition(0);
            break;
        }
    }
    
    return nErr;
}

STInt STIPodLibraryPlugin::FillSampleBuffer(STSampleBuffer* aBuffer)
{
    STUint8* pDstBufferPtr = aBuffer->Ptr() + aBuffer->StartPos();
    STInt nReadSize = aBuffer->ValidSize();
    
    aBuffer->SetStartTime(((iTotalReadSize - aBuffer->StartPos()) * 1000)/(iMediaInfo.iSampleRate * iMediaInfo.iChannels * sizeof(STInt16)));
    
    if (iCurSampleBufferDataPtr != NULL)
    {
        STASSERT(iCurSampleBufferDataOffset < iCurSampleBufferTotalLen);
        STInt nRemainSize = iCurSampleBufferTotalLen - iCurSampleBufferDataOffset;
        if (nReadSize >= nRemainSize)
        {
            memcpy(pDstBufferPtr, iCurSampleBufferDataPtr + iCurSampleBufferDataOffset, nRemainSize);
            iTotalReadSize += nRemainSize;
            iCurSampleBufferDataPtr = NULL;
            iCurSampleBufferDataOffset = 0;
            iCurSampleBufferTotalLen = 0;
            aBuffer->SetPosition(aBuffer->StartPos() + nRemainSize);
            
            if (nReadSize == nRemainSize)
            {
                aBuffer->SetPosition(0);
                return STKErrNone;
            }
        }
        else
        {
            memcpy(pDstBufferPtr, iCurSampleBufferDataPtr + iCurSampleBufferDataOffset, nReadSize);
            iTotalReadSize += nReadSize;
            iCurSampleBufferDataOffset += nReadSize;
            aBuffer->SetPosition(0);
            return STKErrNone;
        }
    }
    return STKErrUnderflow;
}

STInt STIPodLibraryPlugin::ReadNextSampleBuffer()
{
    AVAssetReader* pAssetReader = (AVAssetReader*)iAssetReader;
    AVAssetReaderAudioMixOutput* pReaderOutput = (AVAssetReaderAudioMixOutput*)iAssetReaderOutput;
    
    PreSampleRefRelease();
    
    CMSampleBufferRef pSample = NULL;
    
    if(pAssetReader.status == AVAssetReaderStatusReading)
    {      
        pSample = [pReaderOutput copyNextSampleBuffer];
    }
    
    if (pSample != NULL)
    {        
        CMBlockBufferRef pBuffer = CMSampleBufferGetDataBuffer(pSample);
        
        size_t nLengthAtOffset = 0;
        size_t nTotalLength = 0;
        STChar* pDataPtr;
        
        if(CMBlockBufferGetDataPointer(pBuffer, 0, &nLengthAtOffset, &nTotalLength, &pDataPtr) == noErr)
        {
            iCurSampleBufferDataOffset = 0;
            iCurSampleBufferDataPtr = pDataPtr;
            iCurSampleBufferTotalLen = nTotalLength;
        }
        
        iCurSampleBufferRef = (void*)pSample;
        
        return STKErrNone;
    }  
    
    printf("read status:%d\n", pAssetReader.status);
    
    if (pAssetReader.status == AVAssetReaderStatusCompleted) 
    {
        iReadStatus = ESTReadStatusComplete;
        return STKErrEof;
    }
    else 
    {
        iReadStatus = ESTReadStatusReadErr;
        return STKErrAccessDenied;
    }
}


void STIPodLibraryPlugin::PreSampleRefRelease()
{
    if (iCurSampleBufferRef != NULL)
    {
        CFRelease((CMSampleBufferRef)iCurSampleBufferRef);
        iCurSampleBufferRef = NULL;
    }
}

void STIPodLibraryPlugin::Seek(STUint aTime)
{
    PreSampleRefRelease();
    
    [(AVAssetReaderAudioMixOutput*)iAssetReaderOutput release];
    iAssetReaderOutput = NULL;
    
    [(AVAssetReader*)iAssetReader cancelReading];
    [(AVAssetReader*)iAssetReader release];
    iAssetReader = NULL;      
    
    StartReadingAt(aTime);
}

STInt STIPodLibraryPlugin::StartReadingAt(STUint aTime)
{
    AVAsset* pAsset = (AVAsset*)iAsset;
    STUint nSeekTime = (aTime * 1000 + 500) / 1000;
    iTotalReadSize = nSeekTime;
    iTotalReadSize *= (iMediaInfo.iSampleRate * iMediaInfo.iChannels * sizeof(STInt16));
    iTotalReadSize /= 1000;
    CMTime nToSeekTime = CMTimeMakeWithSeconds(nSeekTime / 1000, [pAsset duration].timescale);
    CMTimeRange tTimerange = CMTimeRangeMake(nToSeekTime, pAsset.duration);
    
    NSError* Error = 0;
    AVAssetReader* pAssetReader = [[AVAssetReader assetReaderWithAsset:pAsset error:&Error] retain]; 
    
    AVAssetReaderAudioMixOutput * readerOutput = [[AVAssetReaderAudioMixOutput assetReaderAudioMixOutputWithAudioTracks:((NSArray*)iAudioArray) audioSettings:nil] retain];
    
    iAssetReaderOutput = (void*)readerOutput;
    iAssetReader = (void*) pAssetReader;
    
    [pAssetReader addOutput:readerOutput];
    
    pAssetReader.timeRange = tTimerange;
    
    if (![pAssetReader startReading])
    {
        //NSLogDebug(@"CTTIPodLibraryFileReader AssetReader TTKErrNotReady");
        return STKErrNotReady;
    }
    
    PreSampleRefRelease();
    iCurSampleBufferDataPtr = NULL;
    
    iReadStatus = ESTReadStatusReading;

    return STKErrNone;
}
