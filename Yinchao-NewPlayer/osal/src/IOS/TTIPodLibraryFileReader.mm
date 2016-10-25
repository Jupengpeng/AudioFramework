/**
* File : TTPureDataReader.cpp
* Created on : 2011-8-31
* Author : hu.cao
* Copyright : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
* Description : CTTPureDataReader 实现文件
*/

// INCLUDES
#include <stdio.h>
#include <string.h>
#include "TTMacrodef.h"
#include "TTIPodLibraryFileReader.h"
#include "TTOSALConfig.h"
#include "TThread.h"
#import <AVFoundation/AVFoundation.h>
#include <AVFoundation/AVAudioSession.h>
#include "TTIPodLibraryPCMOutputFormat.h"
#include "TTAutoReleasePool.h"
#include "TTBackgroundConfig.h"
#include "TTLog.h"
#include "TTUIDeviceHardware.h"

TTBool CTTIPodLibraryFileReader::IsSourceValid(const TTChar* aUrl)
{
    AVURLAsset *assert = [[[AVURLAsset alloc] initWithURL:[NSURL URLWithString:[NSString stringWithUTF8String:aUrl]] options:nil] autorelease];
    return assert.playable;
}

CTTIPodLibraryFileReader::CTTIPodLibraryFileReader()
: iAsset(NULL)
, iAssetReader(NULL)
, iAssetReaderOutput(NULL)
, iCurSampleBufferRef(NULL)
, iDuration(0)
, iReadStarted(ETTFalse)
, iSampleRate(0)
, iChannels(0)
, iTotalReadSize(0)
, iAudioArray(NULL)
{
    
}

CTTIPodLibraryFileReader::~CTTIPodLibraryFileReader()
{
}

TTInt CTTIPodLibraryFileReader::GetSampleRate()
{
    return iSampleRate;
}

TTInt CTTIPodLibraryFileReader::GetChannels()
{
    return iChannels;
}

TTUint CTTIPodLibraryFileReader::Duration()
{
    return (TTUint)(CMTimeGetSeconds(((AVAsset*)iAsset).duration) * 1000);
}

TTInt CTTIPodLibraryFileReader::Open(const TTChar* aUrl)
{ 
    NSString* pStr = [[NSString alloc] initWithUTF8String:aUrl];    
    NSURL* pNewUrl = [[NSURL alloc] initWithString:pStr];
   
    iAsset = (void*)([[AVURLAsset URLAssetWithURL:pNewUrl options:nil] retain]);  
    
    [pStr release];
    [pNewUrl release];
    
    iAudioArray = (void*)[((AVAsset*)iAsset) tracksWithMediaType:AVMediaTypeAudio];
    
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
                iSampleRate = pDesc->mSampleRate;
                iChannels = pDesc->mChannelsPerFrame;
            }
        }       
    }
    
    if ((iSampleRate <= 0) || ((iChannels != 1) && (iChannels != 2)))
    {
        [(NSArray*)iAudioArray release];
        [((AVAsset*)iAsset) release];
        iAudioArray = NULL;
        iAsset = NULL;
        return TTKErrNotSupported;
    }
    
    return TTKErrNone;
}

TTInt CTTIPodLibraryFileReader::Close()
{  
    PreSampleRefRelease();
    
    [(AVAssetReader*)iAssetReader cancelReading];
    
    [(AVAssetReaderAudioMixOutput*)iAssetReaderOutput release];
    iAssetReaderOutput = NULL;
    
    
    [(AVAssetReader*)iAssetReader release];
    iAssetReader = NULL;
    
    [(NSArray*)iAudioArray release];
    iAudioArray = NULL;
    
    [(AVAsset*)iAsset release];
    iAsset = NULL;
    
    iReadStarted = ETTFalse;
    
    iSampleRate = 0;
    iChannels = 0;
    
    iTotalReadSize = 0;
    
	return TTKErrNone;
}

TTInt CTTIPodLibraryFileReader::ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{        
    TTInt nErr = aReadSize;
 
    if (!iReadStarted)
    {
        TTInt nErr = SetStartReadPos(0);
        if (nErr != TTKErrNone)
        {
            return nErr;
        }
        iReadStarted = ETTTrue;
    }
    
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
        TTChar* pDataPtr;
        
        if(CMBlockBufferGetDataPointer(pBuffer, 0, &nLengthAtOffset, &nTotalLength, &pDataPtr) == noErr)
        {
            TTPureDataOutputFormat* pFormat = (TTPureDataOutputFormat*)aReadBuffer;
            pFormat->iCurOffset = 0;
            pFormat->iDataPtr = pDataPtr;
            pFormat->iTotalLen = nTotalLength;
            pFormat->iStartTime = iTotalReadSize * 1000 / (iSampleRate * iChannels * sizeof(TTInt16));
            iTotalReadSize += nTotalLength;
            pFormat->iStopTime = iTotalReadSize * 1000 / (iSampleRate * iChannels * sizeof(TTInt16));
        }
        
        iCurSampleBufferRef = (void*)pSample;
    }  
    else
    {
        if (pAssetReader.status == AVAssetReaderStatusCompleted)
        {
            NSLogDebug(@"Asset Read Complete!");
            nErr = TTKErrEof;
        }
        else if (pAssetReader.status == AVAssetReaderStatusFailed)
        {
            NSLogDebug(@"Asset Read Fail! Status:%d Error:%@", pAssetReader.status,pAssetReader.error);
            if (pAssetReader.error.code == AVErrorOperationInterrupted || (![[TTUIDeviceHardware instance] isSystemVersionLargeEqual6] && pAssetReader.error.code == AVErrorUnknown)) {
                nErr = TTKErrOperationInterrupted;
            }else {
                nErr = TTKErrAccessDenied;
            }
        }
        
        memset(aReadBuffer, 0, sizeof(TTPureDataOutputFormat));
    }
    
	return nErr;
}

TTInt CTTIPodLibraryFileReader::Size() const
{
	return iDuration;
}

TTInt CTTIPodLibraryFileReader::ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
    TTASSERT(ETTFalse);
}

ITTDataReader::TTDataReaderId CTTIPodLibraryFileReader::Id()
{
	return ITTDataReader::ETTDataReaderIdIPodLibraryFile;
}

void CTTIPodLibraryFileReader::PreSampleRefRelease()
{
    if (iCurSampleBufferRef != NULL)
    {
        CFRelease((CMSampleBufferRef)iCurSampleBufferRef);
        iCurSampleBufferRef = NULL;
    }
}

TTInt CTTIPodLibraryFileReader::Seek(TTUint aTime)
{
    PreSampleRefRelease();
    
    [(AVAssetReaderAudioMixOutput*)iAssetReaderOutput release];
    iAssetReaderOutput = NULL;
    
    [(AVAssetReader*)iAssetReader cancelReading];
    [(AVAssetReader*)iAssetReader release];
    iAssetReader = NULL;      
    
    TTInt nErr = SetStartReadPos(aTime);
    iReadStarted = ETTTrue;
    
    return nErr;
}

TTInt CTTIPodLibraryFileReader::SetStartReadPos(TTUint aTime)
{
    //TTASSERT(!iReadStarted);
    AVAsset* pAsset = (AVAsset*)iAsset;
    if ([pAsset tracksWithMediaType:AVMediaTypeAudio] <= 0) {
        return TTKErrNotSupported;
    }
    
    TTUint nSeekTime = aTime;
    iTotalReadSize = nSeekTime;
    iTotalReadSize *= (iSampleRate * iChannels * sizeof(TTInt16));
    iTotalReadSize /= 1000;
    CMTime nToSeekTime = CMTimeMakeWithSeconds(nSeekTime / 1000, [pAsset duration].timescale);
    CMTimeRange tTimerange = CMTimeRangeMake(nToSeekTime, kCMTimePositiveInfinity);
    
    NSError* Error = 0;
    AVAssetReader* pAssetReader = [[AVAssetReader assetReaderWithAsset:pAsset error:&Error] retain]; 
    
    AVAssetReaderAudioMixOutput * readerOutput = [[AVAssetReaderAudioMixOutput assetReaderAudioMixOutputWithAudioTracks:((NSArray*)iAudioArray) audioSettings:nil] retain];
    
    iAssetReaderOutput = (void*)readerOutput;
    iAssetReader = (void*) pAssetReader;
    
    [pAssetReader addOutput:readerOutput];
    
    pAssetReader.timeRange = tTimerange;
    
    if (![pAssetReader startReading])
    {
        NSLogDebug(@"CTTIPodLibraryFileReader AssetReader TTKErrNotReady");
        return TTKErrNotReady;
    }

    return TTKErrNone;
}

TTUint16 CTTIPodLibraryFileReader::ReadUint16(TTInt aReadPos)
{
    return 0;
}

TTUint16 CTTIPodLibraryFileReader::ReadUint16BE(TTInt aReadPos)
{
    return 0;
}

TTUint32 CTTIPodLibraryFileReader::ReadUint32(TTInt aReadPos)
{
    return 0;
}

TTUint32 CTTIPodLibraryFileReader::ReadUint32BE(TTInt aReadPos)
{
    return 0;
}

TTUint64 CTTIPodLibraryFileReader::ReadUint64(TTInt aReadPos)
{
    return 0;
}

TTUint64 CTTIPodLibraryFileReader::ReadUint64BE(TTInt aReadPos)
{
    return 0;
}

TTInt CTTIPodLibraryFileReader::PrepareCache(TTInt aReadPos, TTInt aReadSize, TTInt aFlag)
{
    return 0;
}
