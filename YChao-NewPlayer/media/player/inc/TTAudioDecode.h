#ifndef __TT_TTAUDIODECODER_H__
#define __TT_TTAUDIODECODER_H__

// INCLUDES
#include "TTAudioPlugin.h"
#include "TTMediaPlayerItf.h"
#include "TTMediadef.h"
#include "TTSrcDemux.h"
#include "TTCritical.h"
#include "RIFF.h"
//#define __DUMP1_PCM__

class CTTAudioDecode
{
public:

	CTTAudioDecode(CTTSrcDemux*	aSrcMux, TTInt nTimeStep);
	virtual ~CTTAudioDecode();

public:
	virtual TTInt							initDecode(TTAudioInfo* pCurAudioInfo);

	virtual TTInt							uninitDecode();

	virtual	TTInt							start();

	virtual	TTInt							stop();

	virtual	TTInt							pause();

	virtual	TTInt 							resume();

	virtual TTInt							flush();
	
	virtual TTInt							getOutputBuffer(TTBuffer* DstBuffer);

	virtual TTInt							setParam(TTInt aID, void* pValue);

	virtual TTInt							getParam(TTInt aID, void* pValue);

protected:
	virtual TTInt							updateStep();
	virtual TTInt							updateParam(TTAudioInfo* pCurAudioInfo);
	virtual TTInt							DecBuffer(TTPBYTE pBuffer,	TTInt32	nSize);

private:
	CTTSrcDemux*							mSrcMux;
	CTTAudioPluginManager*					mPluginManager;

	TTBuffer*								mCurBuffer;	
	TTBuffer								mSrcBuffer;
	TTAudioFormat							mAudioFormat;
	TTInt32									mAudioStepTime;
	TTInt32									mAudioStepSize;
	TTInt32									mAudioDecSize;
	TTInt64									mAudioFrameTime;
	RTTCritical								mCritical;
	RTTCritical								mCriStatus;
	TTPlayStatus							mStatus;

	TTUint32								mAudioCodec;
	TTWAVFormat								mWAVFormat;

#ifdef __DUMP1_PCM__
	FILE*									DumpFile;
#endif
};

#endif //__TT_TTCAUDIODECODER_H__
