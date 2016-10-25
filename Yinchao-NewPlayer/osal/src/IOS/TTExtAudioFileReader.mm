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
#include "TTOSALConfig.h"
#include "TThread.h"
#include "TTExtAudioFileReader.h"
#include "TTAutoReleasePool.h"
#include "TTBackgroundConfig.h"
#include "TTLog.h"

#define TryCatchForAnyExceptionWithOnlyTryBlock(tryBlock) \
        TryCatchForAnyException(tryBlock, )

#define TryCatchForAnyException(tryBlock, catchBlock) \
    try {                           \
        tryBlock;                   \
    } catch (...) {                 \
        catchBlock;                 \
    }

TTBool CTTExtAudioFileReader::IsSourceValid(const TTChar* aUrl)
{   
    NSURL* pNewUrl = [NSURL fileURLWithPath:[NSString stringWithUTF8String:aUrl]];
    ExtAudioFileRef audioFileObject = NULL;    
    OSStatus nErr = TTKErrNone;
    
    TryCatchForAnyExceptionWithOnlyTryBlock(nErr = ExtAudioFileOpenURL((CFURLRef)pNewUrl, &audioFileObject))
    
    if (nErr == TTKErrNone)
    {
        TryCatchForAnyExceptionWithOnlyTryBlock(ExtAudioFileDispose(audioFileObject))
    }
 
    return (nErr == TTKErrNone);
}

CTTExtAudioFileReader::CTTExtAudioFileReader()
: iCurAudioFile(NULL)
, iSampleRate(0)
, iChannels(0)
, iReadStarted(ETTFalse)
, iTotalFrameNum(0)
{
}

CTTExtAudioFileReader::~CTTExtAudioFileReader()
{
    
}

TTInt CTTExtAudioFileReader::GetSampleRate()
{
    return iSampleRate;
}

TTInt CTTExtAudioFileReader::GetChannels()
{
    return iChannels;
}

TTInt64 CTTExtAudioFileReader::TotalFrame()
{
    return iTotalFrameNum;
}

TTUint CTTExtAudioFileReader::Duration()
{
    return iDuration;
}

TTInt CTTExtAudioFileReader::Open(const TTChar* aUrl)
{    
    NSURL* pNewUrl = [NSURL fileURLWithPath:[NSString stringWithUTF8String:aUrl]];
    OSStatus nErr = TTKErrNone;
    
    TryCatchForAnyExceptionWithOnlyTryBlock(nErr = ExtAudioFileOpenURL((CFURLRef)pNewUrl, &iCurAudioFile))

    if (nErr == TTKErrNone)
    {
        AudioStreamBasicDescription fileAudioFormat = {0};
        UInt32 formatPropertySize = sizeof (fileAudioFormat);
        
        TryCatchForAnyExceptionWithOnlyTryBlock(nErr = ExtAudioFileGetProperty(iCurAudioFile, kExtAudioFileProperty_FileDataFormat, &formatPropertySize, &fileAudioFormat))
        
        if (nErr == TTKErrNone)
        {
            iSampleRate = fileAudioFormat.mSampleRate;
            iChannels = fileAudioFormat.mChannelsPerFrame;
            
            if ((iSampleRate <= 0) || ((iChannels != 1) && (iChannels != 2)))
            {
                nErr = TTKErrNotSupported;
            }
            else
            {
                SInt64 nFrameNum = 0;
                UInt32 nPropertySize = sizeof(nFrameNum);
                
                TryCatchForAnyExceptionWithOnlyTryBlock(nErr = ExtAudioFileGetProperty(iCurAudioFile, kExtAudioFileProperty_FileLengthFrames, &nPropertySize, &nFrameNum))
                
                if (nErr == TTKErrNone)
                {
                    if (nFrameNum > 0)
                    {
                        iTotalFrameNum = nFrameNum;
                        SInt64 ntmp = nFrameNum;
                        ntmp *= 1000;
                        iDuration = ntmp / iSampleRate;
                    }
                    else
                    {
                        nErr = TTKErrNotSupported;
                    }
                }
            }
        }
    }
    
    if ((nErr != TTKErrNone) && (iCurAudioFile != NULL))
    {
        TryCatchForAnyExceptionWithOnlyTryBlock(ExtAudioFileDispose(iCurAudioFile))
        
        iCurAudioFile = NULL;
        nErr = TTKErrNotSupported;
    }
    else
    {
        TTASSERT(!iReadStarted);
        if (noErr == (nErr = StartReading()))
        {
            iReadStarted = ETTTrue;
        }
        else
        {
            nErr = TTKErrNotSupported;
        }
    }
    
    return nErr;
}

TTInt CTTExtAudioFileReader::Close()
{
    if (iCurAudioFile != NULL)
    {
        TryCatchForAnyExceptionWithOnlyTryBlock(ExtAudioFileDispose(iCurAudioFile))
        iCurAudioFile = NULL;
    }
    iReadStarted = ETTFalse;
	return TTKErrNone;
}

TTInt CTTExtAudioFileReader::ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
    OSStatus nErr = noErr;
    if ((!iReadStarted) && (noErr == (nErr = StartReading())))
    {
        iReadStarted = ETTTrue;                
    }
    
    UInt32 nPackets = 0;
    if (nErr == noErr)
    {
        TTASSERT(iCurBufferList.mNumberBuffers == 1);
        TTASSERT(iCurBufferList.mBuffers[0].mNumberChannels == iChannels);
        iCurBufferList.mBuffers[0].mData = aReadBuffer;
        iCurBufferList.mBuffers[0].mDataByteSize = aReadSize;
    
        nPackets = aReadSize / (iChannels * sizeof(TTInt16));
        
        TryCatchForAnyExceptionWithOnlyTryBlock(nErr = ExtAudioFileRead(iCurAudioFile, &nPackets, &iCurBufferList))
    
        if (nErr != TTKErrNone)
        {
            NSLogDebug(@"ExtReaderFile read error!\n");
        }    
    }
    return (nErr == TTKErrNone) ? nPackets * iChannels * sizeof(TTInt16) : nErr;
}

TTInt CTTExtAudioFileReader::Size() const
{
	return 0;
}

TTInt CTTExtAudioFileReader::ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
    return 0;
}

ITTDataReader::TTDataReaderId CTTExtAudioFileReader::Id()
{
	return ITTDataReader::ETTDataReaderIdExtAudioFile;
}

TTInt CTTExtAudioFileReader::Seek(TTUint aFrameIndex)
{
    if (!iReadStarted)
    {
        if (noErr == StartReading())
        {
            iReadStarted = ETTTrue;
        }        
    }
    
    TTInt64 nSeekFramePos = KReadSizePerTime;
    nSeekFramePos *= aFrameIndex;
    nSeekFramePos /= (sizeof(TTInt16) * iChannels);
 
    TryCatchForAnyExceptionWithOnlyTryBlock(ExtAudioFileSeek(iCurAudioFile, nSeekFramePos))

    return TTKErrNone;
}

TTInt CTTExtAudioFileReader::StartReading()
{
    iCurBufferList.mNumberBuffers = 1;
    iCurBufferList.mBuffers[0].mNumberChannels = iChannels;
    iCurBufferList.mBuffers[0].mData = NULL;
    iCurBufferList.mBuffers[0].mDataByteSize = 0;
    
    AudioStreamBasicDescription tClientFormat = {0};
    tClientFormat.mSampleRate = iSampleRate;
    tClientFormat.mFormatID = kAudioFormatLinearPCM;  
    tClientFormat.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
    tClientFormat.mBytesPerPacket = sizeof(TTInt16) * iChannels;
    tClientFormat.mFramesPerPacket = 1;   
    tClientFormat.mBytesPerFrame = sizeof(TTInt16) * iChannels;  
    tClientFormat.mChannelsPerFrame = iChannels;  
    tClientFormat.mBitsPerChannel = 16;
    
    OSStatus error = TTKErrNone;
    
    TryCatchForAnyExceptionWithOnlyTryBlock(error = ExtAudioFileSetProperty(iCurAudioFile, kExtAudioFileProperty_ClientDataFormat, sizeof(tClientFormat), &tClientFormat))
    
    return error;
}

TTUint16 CTTExtAudioFileReader::ReadUint16(TTInt aReadPos)
{
    return 0;
}

TTUint16 CTTExtAudioFileReader::ReadUint16BE(TTInt aReadPos)
{
    return 0;
}

TTUint32 CTTExtAudioFileReader::ReadUint32(TTInt aReadPos)
{
    return 0;
}

TTUint32 CTTExtAudioFileReader::ReadUint32BE(TTInt aReadPos)
{
    return 0;
}

TTUint64 CTTExtAudioFileReader::ReadUint64(TTInt aReadPos)
{
    return 0;
}

TTUint64 CTTExtAudioFileReader::ReadUint64BE(TTInt aReadPos)
{
    return 0;
}

TTInt CTTExtAudioFileReader::PrepareCache(TTInt aReadPos, TTInt aReadSize, TTInt aFlag)
{
    //if(aReadPos > iFileSize || aReadPos + aReadSize > iFileSize)
    //    return TTKErrOverflow;
    
    return TTKErrNone;
}
