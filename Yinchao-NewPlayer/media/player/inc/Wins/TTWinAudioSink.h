#ifndef __TT_WIN_AUDIO_SINK_H__
#define __TT_WIN_AUDIO_SINK_H__

//#define __DUMP_PCM__
#include "stdio.h"

// INCLUDES
#include "TTBaseAudioSink.h"
#include "TTCritical.h"

// CLASSES DECLEARATION
class CTTWinAudioSink : public TTCBaseAudioSink
{
public:
	/**
	* \fn							CTTAudioSink(ITTSinkDataProvider* aDataProvider, ITTPlayRangeObserver& aObserver);
	* \brief						���캯��
	* \param[in] aDataProvider		�����ṩ�߽ӿ�����
	* \param[in] aObserver			�ص�
	*/
	CTTWinAudioSink(CTTSrcDemux* SrcMux, TTInt nCount);

	/**
	* \fn							~CTTAudioSink();
	* \brief						��������
	*/
	virtual ~CTTWinAudioSink();


public://from ITTDataSink

	virtual TTInt				render();

	virtual	TTInt				stop();

	virtual TTInt				flush();

	virtual TTInt				newAudioTrack();
	virtual TTInt				closeAudioTrack();

	virtual	TTInt				syncPosition(TTUint64 aPosition, TTInt aOption = 0);

	virtual TTInt				setVolume(TTInt aLVolume, TTInt aRVolume);

	virtual TTInt				AudioDone(void* param);	
private:

	TTInt						WriteData(TTBuffer* aBuffer);


	TTInt						mAvgBytesPerSec;

	bool						mSetProirity;

	RTTCritical					mCritRender;

private:
#ifdef __DUMP_PCM__
	FILE*						mDumpFile;
#endif
};

#endif // __TT_WIN_AUDIO_SINK_H__