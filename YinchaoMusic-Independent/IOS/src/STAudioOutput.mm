#include "STAudioOutput.h"
#include "STOSConfig.h"
#include "STSampleBuffer.h"
#include "STThread.h"
#include <stdio.h>
#include "STBackgroundConfig.h"

STAudioOutput::STAudioOutput(ISTPlayRangeOverflowObserver& aObserver)
: STBaseAudioOutput(aObserver)
, iEQPresetsArray(NULL)
, iEQPresetIdx(0)
{
	STBackgroundAudioQueueConfig::EnableBackground(ESTTrue);
}

STAudioOutput::~STAudioOutput()
{
//    printf("STAudioOutput::~STAudioOutput()\n");
}

void STAudioOutput::Pause()
{
	AudioUnitStop();
}

void STAudioOutput::Resume()
{
    AudioUnitStart();
}

STInt STAudioOutput::Open(STInt aSampleRate, STInt aChannels)
{
	STBaseAudioOutput::Open(aSampleRate, aChannels);
    
    AudioStreamBasicDescription tOutputFormat;
    STInt nSampleSize = sizeof(STInt16);
    tOutputFormat.mFormatID = kAudioFormatLinearPCM;
    tOutputFormat.mFormatFlags = kAudioFormatFlagsCanonical;
    tOutputFormat.mBitsPerChannel = 8 * nSampleSize;
    tOutputFormat.mChannelsPerFrame = iChannels;
    tOutputFormat.mFramesPerPacket = 1;
    tOutputFormat.mBytesPerPacket = tOutputFormat.mBytesPerFrame = iChannels * nSampleSize;
    tOutputFormat.mSampleRate = aSampleRate;
    
    OSStatus nErr = noErr;
    nErr = NewAUGraph(&iAudioGraph);
    if (nErr != STKErrNone)
        return STKErrAccessDenied;
    
    // output unit
    AudioComponentDescription output_desc;
    output_desc.componentType = kAudioUnitType_Output;
	output_desc.componentSubType = kAudioUnitSubType_RemoteIO;
	output_desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	output_desc.componentFlags = 0;
	output_desc.componentFlagsMask = 0;
    
    // iPodEQ unit
    AudioComponentDescription eq_desc;
    eq_desc.componentType = kAudioUnitType_Effect;
    eq_desc.componentSubType = kAudioUnitSubType_AUiPodEQ;
	eq_desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	eq_desc.componentFlags = 0;
	eq_desc.componentFlagsMask = 0;
    
    // iPodEQ unit
    AudioComponentDescription input_desc;
    input_desc.componentType = kAudioUnitType_FormatConverter;
    input_desc.componentSubType = kAudioUnitSubType_AUConverter;
	input_desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	input_desc.componentFlags = 0;
	input_desc.componentFlagsMask = 0;
    
    AUNode tOutputNode;
    nErr = AUGraphAddNode(iAudioGraph, &output_desc, &tOutputNode);
	if (nErr != noErr)
        return STKErrAccessDenied;
    
    AUNode tEQNode;
    nErr = AUGraphAddNode(iAudioGraph, &eq_desc, &tEQNode);
	if (nErr != noErr)
        return STKErrAccessDenied;
    
    AUNode tInputNode;
    nErr = AUGraphAddNode(iAudioGraph, &input_desc, &tInputNode);
	if (nErr != noErr)
        return STKErrAccessDenied;
    
    nErr = AUGraphConnectNodeInput(iAudioGraph, tInputNode, 0, tEQNode, 0);
    if (nErr != noErr) 
        return STKErrAccessDenied;
    
    nErr = AUGraphConnectNodeInput(iAudioGraph, tEQNode, 0, tOutputNode, 0);
    if (nErr != noErr) 
        return STKErrAccessDenied;
    
	nErr = AUGraphOpen(iAudioGraph);
	if (nErr != noErr)
        return STKErrAccessDenied;
    
  
    nErr = AUGraphNodeInfo(iAudioGraph, tInputNode, NULL, &iInputUnit);
    if (nErr != noErr)
        return STKErrAccessDenied;
    
	nErr = AUGraphNodeInfo(iAudioGraph, tEQNode, NULL, &iEQUnit);
    if (nErr != noErr)
        return STKErrAccessDenied;
    
    nErr = AUGraphNodeInfo(iAudioGraph, tOutputNode, NULL, &iOutputUnit);
    if (nErr != noErr)
        return STKErrAccessDenied;
    
    AURenderCallbackStruct rcbs;
    rcbs.inputProc = &AudioUnitCallBack;
    rcbs.inputProcRefCon = this;
    
    UInt32 size = sizeof(iEQPresetsArray);
    nErr = AudioUnitGetProperty(iEQUnit, kAudioUnitProperty_FactoryPresets, kAudioUnitScope_Global, 0, &iEQPresetsArray, &size);
    if (nErr != noErr)
        return STKErrAccessDenied;  
    
    UInt32 maxFPS = 8192;
    nErr = AudioUnitSetProperty(iInputUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &maxFPS, sizeof(maxFPS));
    if (nErr != noErr)
        return STKErrAccessDenied;
    
    nErr = AudioUnitSetProperty(iEQUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &maxFPS, sizeof(maxFPS));
    if (nErr != noErr)
        return STKErrAccessDenied;
    
    maxFPS = 8192 * 2;
    nErr = AudioUnitSetProperty(iOutputUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &maxFPS, sizeof(maxFPS));
    if (nErr != noErr)
        return STKErrAccessDenied;
    
    nErr = AUGraphSetNodeInputCallback(iAudioGraph, tInputNode, 0, &rcbs);
    if (nErr != noErr)
        return STKErrAccessDenied;
    
    nErr = AudioUnitSetProperty(iInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &tOutputFormat, sizeof(tOutputFormat));
    if (nErr != noErr)
        return STKErrAccessDenied;
    
    nErr = AUGraphInitialize(iAudioGraph);
    if (nErr != STKErrNone)
        return STKErrAccessDenied;
    
//    printf("STAudioOutput::Open()\n");
    
    SetEqualizerPresetIdx(iEQPresetIdx);
    
    initAudioEffect(aChannels, aSampleRate);
    
	return STKErrNone;
}

void STAudioOutput::Close()
{	
//    printf("STAudioOutput::Close()\n");
    AUGraphClose(iAudioGraph);
    DisposeAUGraph(iAudioGraph);
    iCritical.Lock();
    CFRelease(iEQPresetsArray);
    iEQPresetsArray = NULL;
    iCritical.UnLock();
}

void STAudioOutput::Start()
{
    AudioUnitStart();
}

void STAudioOutput::Stop()
{
//    printf("STAudioOutput::Stop()\n");
    STBaseAudioOutput::Stop();
    AudioUnitStop();
    Flush();
}

void STAudioOutput::AudioUnitStart()
{
    AUGraphStart(iAudioGraph); 
    STBackgroundAudioQueueConfig::EnableBackground(ESTFalse);
}

void STAudioOutput::AudioUnitStop()
{
    STBackgroundAudioQueueConfig::EnableBackground(ESTTrue);
    Boolean bIsRunning = ESTFalse;    
    AUGraphIsRunning(iAudioGraph, &bIsRunning);
    if(bIsRunning)
    {
        AUGraphStop(iAudioGraph); 
    }
}

void STAudioOutput::Flush()
{
	STBaseAudioOutput::Flush();
    iCritical.Lock();
    if (iCurSampleBuffer != NULL)
    {
        iEmptiedBufferArray.Append(iCurSampleBuffer);
        iCurSampleBuffer = NULL;
    }
    iCritical.UnLock();
}

STBool STAudioOutput::IsUnderflow()
{
    iCritical.Lock();
    STBool bUnderflow = iCurSampleBuffer == NULL;
    iCritical.UnLock();
	return bUnderflow;
}

STInt STAudioOutput::GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels)
{
    iCritical.Lock();
    aChannels = iChannels;
    STInt nSize = aSamples * aChannels * sizeof(STInt16);
    if (iCurSampleBuffer != NULL)
    {
        STInt nOffset = iCurSampleBuffer->StartPos();
        STInt nTotoalSize = iCurSampleBuffer->TotalSize();
        if (nSize > nTotoalSize - nOffset)
        {
            nOffset = nTotoalSize - nSize;
        }
        
        memcpy(aWave, iCurSampleBuffer->Ptr() + nOffset, nSize);
    }
    else
    {
        memset(aWave, 0, nSize);
    }
    iCritical.UnLock();
	return STKErrNone;
}

OSStatus STAudioOutput::AudioUnitCallBack(void *inRefCon, AudioUnitRenderActionFlags *
                                         ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 
                                         inBusNumber, UInt32 inNumberFrames, AudioBufferList *
                                         ioData)
{
    STAudioOutput* pAudioOutput = (STAudioOutput*)inRefCon;
    pAudioOutput->AudioUnitCallBackProcL(ioData);
    return noErr;
}

void STAudioOutput::AudioUnitCallBackProcL(AudioBufferList *
                                          ioData)
{   
    STUint8* pDstDataPtr = (STUint8*)ioData->mBuffers[0].mData;
    STInt nRenderSize = ioData->mBuffers[0].mDataByteSize;
    
    iCritical.Lock();
    if (iCurSampleBuffer == NULL)
    {        
        iCurSampleBuffer = GetFilledBuffer();
    }
    
    if (iCurSampleBuffer == NULL)
    {
        iCritical.UnLock();
        memset(pDstDataPtr, 0 , nRenderSize);
        NSLog(@"UnderFlow 210");
        return;
    }
    
    STUint nSrcOffset = iCurSampleBuffer->StartPos();
    
    STUint8* pSrcDataPtr = iCurSampleBuffer->Ptr() + nSrcOffset;
    STInt nValidSize = iCurSampleBuffer->ValidSize();
    if (nValidSize >= nRenderSize) 
    {
        memcpy(pDstDataPtr, pSrcDataPtr, nRenderSize);  
        iCurSampleBuffer->SetPosition(nSrcOffset + nRenderSize);  
        if (nValidSize == nRenderSize)
        {
            iCurSampleBuffer = GetFilledBuffer();
        }
    }
    else
    {
        memcpy(pDstDataPtr, pSrcDataPtr, nValidSize);
        
        iCurSampleBuffer = GetFilledBuffer();
        
        STInt nRemainSize = nRenderSize - nValidSize;
        
        if (iCurSampleBuffer != NULL)
        {
            memcpy(pDstDataPtr + nValidSize, iCurSampleBuffer->Ptr() + iCurSampleBuffer->StartPos(), nRemainSize);
            iCurSampleBuffer->SetPosition(nRemainSize + iCurSampleBuffer->StartPos());
        }
        else
        {
            memset(pDstDataPtr + nValidSize, 0, nRemainSize);
            NSLog(@"UnderFlow 243");
        }
    }      
    
    iCritical.UnLock();
    
    processAudioEffect(pDstDataPtr, nRenderSize);
}

STSampleBuffer*	STAudioOutput::GetFilledBuffer()
{
    if (iCurSampleBuffer != NULL)
    {
        iEmptiedBufferArray.Append(iCurSampleBuffer);
        iCurSampleBuffer = NULL;
    }
    
    STSampleBuffer* pBuffer = NULL;
	if (iFillBufferArray.Count() > 0)
	{
		pBuffer = iFillBufferArray[0];
		iFillBufferArray.Remove(0);
	}
    
    iPluginThreadHandle->LockContext();
    iPluginThreadHandle->Resume();
    iPluginThreadHandle->UnlockContext();
    
    if (pBuffer != NULL)
    {
        STInt nErr = ProcessPlayRange(pBuffer);
        if (STKErrUnderflow == nErr) 
        {
            iEmptiedBufferArray.Append(pBuffer);
            pBuffer = NULL;
        }
        else if (STKErrOverflow == nErr)
        {
            iEmptiedBufferArray.Append(pBuffer);
            pBuffer = NULL;
            iIsOutOfPlayRange = ESTTrue;
        }        
    }
    
	return pBuffer;
}

void STAudioOutput::Position(STUint& aPosition)
{
    iCritical.Lock();  
    if (iCurSampleBuffer != NULL)
    {        
        iCurPosition = iCurSampleBuffer->StartTime() + iCurSampleBuffer->StartPos() * 1000 / (iSampleRate * iChannels * sizeof(STInt16));  
    }
    iCritical.UnLock();
    
    STBaseAudioOutput::Position(aPosition);
}

void STAudioOutput::GetEqualizerPresetArray(STPointerArray<STChar>& aArray)
{
    iCritical.Lock();
    if (iEQPresetsArray != NULL) 
    {
        int nCount = CFArrayGetCount(iEQPresetsArray);  
        for (int nIdx = 0; nIdx < nCount; nIdx++) 
        {
            const STChar* pSrcPresetName = [(NSString*)(((AUPreset*)CFArrayGetValueAtIndex(iEQPresetsArray, nIdx))->presetName) UTF8String];
            STChar* presetName = new STChar[strlen(pSrcPresetName) + 1];
            strcpy(presetName, pSrcPresetName);
            aArray.Append(presetName);
        }
    }
    iCritical.UnLock();
}

void STAudioOutput::SetEqualizerPresetIdx(STInt aIdx)
{
    iEQPresetIdx = aIdx;
    if (iEQPresetsArray != NULL)
    {
        AUPreset *preset = (AUPreset*)CFArrayGetValueAtIndex(iEQPresetsArray, iEQPresetIdx);
        AudioUnitSetProperty(iEQUnit, kAudioUnitProperty_PresentPreset, kAudioUnitScope_Global, 0, preset, sizeof(AUPreset));
    }
}

void STAudioOutput::initAudioEffect(STInt channels, STInt sampleRate)
{
    iDelayEffect.Open(channels, sampleRate);
    iBassBoost.Open(channels, sampleRate);
    i3DChorus.Open(sampleRate);
}

void STAudioOutput::processAudioEffect(void* buf, STInt bufSize)
{
    iCritical.Lock();
    iDelayEffect.Process(buf, bufSize);
    iBassBoost.Process(buf, bufSize);
    i3DChorus.Process(buf, bufSize);
    iSurrondEffect.Process(buf, bufSize);
    iCritical.UnLock();
}

void STAudioOutput::EnableEffect(STEffectType aEffect, STInt aParam1, STInt aParam2)
{
    iCritical.Lock();
    switch (aEffect)
    {
        case ESTEffectTypeSurround:
            iSurrondEffect.Enable();
            iSurrondEffect.SetParameter(aParam1);
            break;
            
        case ESTEffectTypeBassBoost:
            iBassBoost.SetLevel(aParam1);
            break;
            
        case ESTEffectType3DChorus:
            i3DChorus.Enable();
            i3DChorus.SetParameter(aParam1);
            break;
            
        case ESTEffectTypeDelay:
            iDelayEffect.Enable();
            iDelayEffect.SetParameter(aParam1, aParam2);
            break;
        
        default:
            break;
    }
    iCritical.UnLock();
}

void STAudioOutput::DisableEffect(STEffectType aEffect)
{
    iCritical.Lock();
    switch (aEffect)
    {
        case ESTEffectTypeSurround:
            iSurrondEffect.Disable();
            break;
            
        case ESTEffectTypeBassBoost:
            iBassBoost.SetLevel(0);
            break;
            
        case ESTEffectType3DChorus:
            i3DChorus.Disable();
            break;
            
        case ESTEffectTypeDelay:
            iDelayEffect.Disable();
            break;
            
        default:
            break;
    }
    iCritical.UnLock();
}

