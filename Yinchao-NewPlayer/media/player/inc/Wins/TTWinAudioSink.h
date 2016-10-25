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
	* \brief						构造函数
	* \param[in] aDataProvider		数据提供者接口引用
	* \param[in] aObserver			回调
	*/
	CTTWinAudioSink(CTTSrcDemux* SrcMux, TTInt nCount);

	/**
	* \fn							~CTTAudioSink();
	* \brief						析构函数
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