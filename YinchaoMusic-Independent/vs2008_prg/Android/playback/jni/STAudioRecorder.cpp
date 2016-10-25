﻿#include <string.h>
#include "STAudioRecorder.h"
#include "STAudioOutput.h"
#include "Resample_samplerate.h"
#include "STBufferConfig.h"

#define KRenderBufferSize (64 * KILO)
#define KRecordSampleRate  (16000)//16000, 11025
#define KRecordBufferTotalSize (KRecordBufferNum * KRecordBufferSize)


STAudioRecorder::STAudioRecorder(STOpenSLESEngine& aEngine, ISTAudioRecorderCallback& aCallback) 
: iOpenSLESEngine(aEngine)
, iCallback(aCallback)
, iRecorderObjectItf(NULL)
, iRecordByteOffset(0)
{
	iRecordBufferQueue = new STInt16[KRecordBufferTotalSize];
	iRecordCurBuffer = iRecordBufferQueue;

	iCurRenderBufferPtr = new STInt16[KRenderBufferSize];

	memset(&iSrcData, 0, sizeof(iSrcData));
}

STAudioRecorder::STAudioRecorder(STOpenSLESEngine& aEngine, ISTAudioRecorderCallback& aCallback,STAudioOutput *stAudio)
{
	STAudioRecorder(aEngine,aCallback);
	pParentAudioOutput = stAudio;

}

STAudioRecorder::~STAudioRecorder()
{
	SAFE_DELETE(iRecordBufferQueue);
	SAFE_DELETE(iCurRenderBufferPtr);
}

STInt STAudioRecorder::Open(STInt aSampleRate, STInt aChannel)
{
	iSampleRate = aSampleRate;
	iChannel = aChannel;

	SLresult result;

	SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
			SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
	SLDataSource audioSrc = {&loc_dev, NULL};

	SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, KRecordBufferNum};

	SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, KRecordSampleRate == 16000 ? SL_SAMPLINGRATE_16 : SL_SAMPLINGRATE_11_025,
		SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
		SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
	SLDataSink audioSnk = {&loc_bq, &format_pcm};

	// create audio recorder
	const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
	const SLboolean req[1] = {SL_BOOLEAN_TRUE};
	result = (*(iOpenSLESEngine.GetEngineItf()))->CreateAudioRecorder(iOpenSLESEngine.GetEngineItf(), &iRecorderObjectItf, &audioSrc,
			&audioSnk, 1, id, req);
	if (SL_RESULT_SUCCESS != result) {
		return STKErrUnknown;
	}

	// realize the audio recorder
	result = (*iRecorderObjectItf)->Realize(iRecorderObjectItf, SL_BOOLEAN_FALSE);
	if (SL_RESULT_SUCCESS != result) {
		return STKErrUnknown;
	}

	// get the record interface
	result = (*iRecorderObjectItf)->GetInterface(iRecorderObjectItf, SL_IID_RECORD, &iRecorderItf);
	STASSERT(SL_RESULT_SUCCESS == result);

	// get the buffer queue interface
	result = (*iRecorderObjectItf)->GetInterface(iRecorderObjectItf, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
			&iRecorderBufferQueueItf);
	STASSERT(SL_RESULT_SUCCESS == result);

	// register callback on the buffer queue
	result = (*iRecorderBufferQueueItf)->RegisterCallback(iRecorderBufferQueueItf, RecorderCallback, this);
	STASSERT(SL_RESULT_SUCCESS == result);

	for(int i = 0; i < KRecordBufferNum; i++)
	{
		(*iRecorderBufferQueueItf)->Enqueue(iRecorderBufferQueueItf, iRecordCurBuffer + i * KRecordBufferSize, KRecordBufferSize*sizeof(STInt16));
	}

	int error = 0;
	if ((iSrcState = src_new (SRC_LINEAR, aChannel, &error)) == NULL)
	{
		return STKErrAbort;
	}

	iSrcData.src_ratio = (STDouble(aSampleRate)) / KRecordSampleRate;
	iSrcData.end_of_input = 0;

	iResampleTempSrcBuffer = (STFloat*)malloc(KRecordBufferSize * sizeof(STFloat) * aChannel);
	memset(iResampleTempSrcBuffer, 0, KRecordBufferSize * sizeof(STFloat) * aChannel);

	iResampleTempDstBuffer = (STFloat*)malloc(KRecordBufferSize * (int)(iSrcData.src_ratio + 1) * sizeof(STFloat) * aChannel);
	memset(iResampleTempDstBuffer, 0, KRecordBufferSize * (int)(iSrcData.src_ratio + 1) * sizeof(STFloat) * aChannel);

	return STKErrNone;
}

void STAudioRecorder::Start()
{
    SLresult result = (*iRecorderItf)->SetRecordState(iRecorderItf, SL_RECORDSTATE_RECORDING);
    STASSERT(SL_RESULT_SUCCESS == result);
}

void STAudioRecorder::Pause()
{
	SLresult result = (*iRecorderItf)->SetRecordState(iRecorderItf, SL_RECORDSTATE_PAUSED);
	STASSERT(SL_RESULT_SUCCESS == result);
}
	
void STAudioRecorder::Resume()
{
    SLresult result = (*iRecorderItf)->SetRecordState(iRecorderItf, SL_RECORDSTATE_RECORDING);
    STASSERT(SL_RESULT_SUCCESS == result);
}

void STAudioRecorder::Stop()
{
    SLresult result = (*iRecorderItf)->SetRecordState(iRecorderItf, SL_RECORDSTATE_STOPPED);
    STASSERT(SL_RESULT_SUCCESS == result);
    result = (*iRecorderBufferQueueItf)->Clear(iRecorderBufferQueueItf);
    STASSERT(SL_RESULT_SUCCESS == result);
}

void STAudioRecorder::Close()
{
    if (iRecorderObjectItf != NULL) {
        (*iRecorderObjectItf)->Destroy(iRecorderObjectItf);
        iRecorderObjectItf = NULL;
        iRecorderItf = NULL;
        iRecorderBufferQueueItf = NULL;
    }	

    SAFE_FREE(iResampleTempSrcBuffer);
    SAFE_FREE(iResampleTempDstBuffer);
    src_delete(iSrcState);
}

void STAudioRecorder::RecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	STASSERT(NULL != context);
	((STAudioRecorder *)context)->CallBack();
}

void STAudioRecorder::CallBack()//录制回调，（得到录制的数据回调至上层）
{
	//add by bin.liu在这里加入降噪处理
	pParentAudioOutput->iDenoise->Process((char*)iRecordCurBuffer,KRecordBufferSize*2,NULL/*转入null表示，iRecordLoopBufferStartPtr 作为降噪处理返回值*/);

	int ret = ProcessResample(iRecordCurBuffer, KRecordBufferSize);
	(*iRecorderBufferQueueItf)->Enqueue(iRecorderBufferQueueItf, iRecordCurBuffer, KRecordBufferSize*sizeof(STInt16));
	iRecordCurBuffer += KRecordBufferSize;
	if(iRecordCurBuffer >= iRecordBufferQueue + KRecordBufferTotalSize)
	{
		iRecordCurBuffer = iRecordBufferQueue;
	}

	STUint state;
	STInt nRenderSize = ret * iChannel * 2;
	STInt nProcessSize = iCallback.RecorderBufferFilled((STUint8 *)iCurRenderBufferPtr, nRenderSize, iRecordByteOffset, state);
	iRecordByteOffset += nProcessSize;
	if (nProcessSize < nRenderSize && state != SL_PLAYSTATE_PLAYING)
	{
		STLOGE("Pause");
		Pause();
	}
}

int STAudioRecorder::ProcessResample(STInt16* aSrc, STInt aFrameNum)
{
	if (iChannel == 2)
	{
		for (int i = 0; i < aFrameNum; i++)
		{
			*(iResampleTempSrcBuffer + (i << 1)) = *(aSrc + i);
			*(iResampleTempSrcBuffer + (i << 1) + 1) = *(aSrc + i);
		}
	}
	else
	{
		for (int i = 0; i < aFrameNum; i++)
		{
			*(iResampleTempSrcBuffer + i) = *(aSrc + i);
		}
	}

	iSrcData.input_frames = aFrameNum;
	iSrcData.data_in = iResampleTempSrcBuffer;

	iSrcData.output_frames = (aFrameNum * iSrcData.src_ratio + 1);
	iSrcData.data_out = iResampleTempDstBuffer;

	if ((0 != src_process (iSrcState, &iSrcData)))
	{
		return 0;
	}

	STInt nCount = iSrcData.output_frames_gen * iChannel;
	for(int i = 0; i < nCount; i++)
	{
		iCurRenderBufferPtr[i]  =  iResampleTempDstBuffer[i];
	}

	return iSrcData.output_frames_gen;
}