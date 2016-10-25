#ifndef __TT_TTAUDIOPROCESS_H__
#define __TT_TTAUDIOPROCESS_H__

//#define __DUMP2_PCM__

#include "TTList.h"
#include "TTCritical.h"
#include "TTEventThread.h"
#include "TTMediainfoDef.h"
#include "TTMediaPlayerItf.h"
#include "TTMediadef.h"
#include "TTSrcDemux.h"
#include "TTAudioDecode.h"
#include "aflibconverter.h"
//#include "TTAudioEffectManager.h"

class TTCAudioProcess
{
public:

	/**
	* \fn									 TTCAudioProcess();
	* \brief								 构造函数
	*/
	TTCAudioProcess(CTTSrcDemux* SrcMux, TTInt nAudioCount = 1);


	/**
	* \fn									~TTCAudioProcess()
	* \brief								析构函数
	*/
	virtual ~TTCAudioProcess();

public:
	virtual TTInt							initProc(TTAudioInfo* pCurAudioInfo);

	virtual TTInt							uninitProc();

	virtual	TTInt							start();

	virtual	TTInt							stop();

	virtual	TTInt							pause();

	virtual	TTInt 							resume();

	virtual TTInt							flush();

	virtual TTInt							getOutputBuffer(TTBuffer* DstBuffer);

	virtual TTInt							setParam(TTInt aID, void* pValue);

	virtual TTInt							getParam(TTInt aID, void* pValue);

	virtual	TTInt							getCurWave(TTInt64 aPlayingTime, TTInt aSamples, TTInt16* aWave, TTInt& aChannels);

	virtual	TTInt							syncPosition(TTUint64 aPosition, TTInt aOption = 0);

	virtual TTInt							onAudioProc (TTInt nMsg, TTInt nVar1, TTInt nVar2, void* nVar3);
	virtual TTInt							postAudioProcEvent (TTInt  nDelayTime);

private:

	virtual	TTInt							allocBuffer();
	virtual	TTInt							freeBuffer();
	virtual TTInt							updateParam();

	virtual	TTInt							allocWaveBuffer();
	virtual	TTInt							freeWaveBuffer();
	virtual	TTInt							updateWaveBuffer(TTBuffer* dstBuffer);

	virtual TTInt							doDecodeFrames(TTBuffer* dstBuffer);
	virtual TTInt							doSampleConvert(TTBuffer* dstBuffer);
	virtual TTInt							doAudioProcess(TTBuffer* dstBuffer);

	virtual void							convert64BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer);
	virtual void							convert8BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer);
	virtual void							convert24BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer);
	virtual void							convert32BitIntTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer);
	virtual void							convert32BitFloatTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer);

	virtual TTInt							doChannelDoMix(TTBuffer* srcBuffer, TTBuffer* dstBuffer);
	virtual TTInt							doSampleBitsConv(TTBuffer* srcBuffer, TTBuffer* dstBuffer);
	virtual TTInt							doSampleRateConv(TTBuffer* srcBuffer, TTBuffer* dstBuffer);

private:
	CTTSrcDemux*							mSrcMux;
	CTTAudioDecode*							mDecoder;

	TTBuffer								mPCMBuffer;
	TTBuffer*								mSrcBuffer;

	TTPlayStatus							mStatus;

	TTPBYTE									mPCMBuf;
	TTUint									mPCMBufSize;
	TTPBYTE									mSinkBuf;
	TTUint									mSinkBufSize;

	TTInt64									mCurTime;
	TTInt64									mLastSysTime;

	TTAudioFormat							mAudioFormatIn;
	TTAudioFormat							mAudioFormatOut;

	TTBuffer**								mListBuffer;
	RTTCritical								mCriList;
	TTInt									mListFull;
	TTInt									mListUsing;
	TTBool									mFlushing;
	TTBool									mSeeking;
	TTBool									mEOS;
	TTBool									mSetProirity;
	TTBool									mEffectEnable;

	TTPBYTE									mWaveBuf;
	TTBuffer**								mWaveBuffer;
	RTTCritical								mWaveList;
	TTInt									mWaveFull;
	TTInt									mWaveUsing;
	TTInt									mDoWave;

	double									mResmpFactor;
	aflibConverter*							mResampleObj;
	TTInt									mSampleRateMAX;
	
	TTInt									mSampleRateSet;
	TTInt									mChannelSet;
	TTInt									mAudioStepTime;
	TTInt									mAudioBufferSize;
	TTInt									mAudioEffectLowDelay;

	RTTCritical								mCritical;
	RTTCritical								mCriEvent;

	TTBuffer*								mCurBuffer;
	
	TTUint									mAudioProcNum;
	TTUint									mAudioCount;
	TTEventThread*							mProcThread;
	TTInt									mDoConvert;
	TTBool									mDoEffect;

#ifdef __DUMP2_PCM__
	FILE*									DumpFile;
#endif
};

class TTCAudioProctEvent : public TTBaseEventItem
{
public:
    TTCAudioProctEvent(TTCAudioProcess * pProc, TTInt (TTCAudioProcess::* method)(TTInt, TTInt, TTInt, void*),
					    TTInt nType, TTInt nMsg = 0, TTInt nVar1 = 0, TTInt nVar2 = 0, void* nVar3 = 0)
		: TTBaseEventItem (nType, nMsg, nVar1, nVar2, nVar3)
	{
		mAudioProc = pProc;
		mMethod = method;
    }

    virtual ~TTCAudioProctEvent()
	{
	}

    virtual void fire (void) 
	{
        (mAudioProc->*mMethod)(mMsg, mVar1, mVar2, mVar3);
    }

protected:
    TTCAudioProcess *		mAudioProc;
    int (TTCAudioProcess::* mMethod) (TTInt, TTInt, TTInt, void*);
};


#endif //__TT_TTAUDIOPOSTPROC_H__
