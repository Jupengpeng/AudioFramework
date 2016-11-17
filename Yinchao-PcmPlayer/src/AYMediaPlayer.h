/*!
* \class CAYMediaPlayer
*
* \brief 
*
* \author cbin
* \date ÎåÔÂ 2016
*/
#ifndef __AYMEDIAPLAYER_H__
#define __AYMEDIAPLAYER_H__

#include <queue>
#include "ulu_thread.h"
#include "IAYMediaPlayer.h"
#include "ulu_OSFunc.h"
#include "ulu_mutex.h"
#include "IAudioRender.h"
#include "STDataReaderItf.h"
#include "STFileReader.h"


#define FAST_PLAY_MODE_SPEED  1.2f

typedef enum {
	AYMEDIAPLAYERTHREADTYPE_MAIN = 1,
	AYMEDIAPLAYERTHREADTYPE_VIDEO = 2,
	AYMEDIAPLAYERTHREADTYPE_AUDIO = 3,
	AYMEDIAPLAYERTHREADTYPE_MAX	  = 0x7FFFFFFF,
}AYMEDIAPLAYERTHREADTYPE;

class CAYMediaPlayer;
class CAYMediaPlayerThread:public ulu_thread{
public:
	CAYMediaPlayerThread(CAYMediaPlayer*pMediaPlayer);
	virtual ~CAYMediaPlayerThread();

protected:
	virtual void ut_thread_function();

protected:
	CAYMediaPlayer* m_pMediaPlayer;
	AYMEDIAPLAYERTHREADTYPE m_eType;

};

class CAYMediaPlayer:public IAYMediaPlayer
{
	friend class CAYMediaPlayerThread;
public:
	CAYMediaPlayer();
	virtual ~CAYMediaPlayer();

public:
	virtual int 			run();

	virtual int				pause();

	virtual int				resume();

	virtual int				stop();

	virtual int				seekTo(unsigned int *uPosition);

	virtual int				getCurrentPosition(unsigned int * puPosition);

	virtual int				setParam(unsigned int uParamId, void * pParam);

	virtual int				getParam(unsigned int uParamId, void * pParam);

	virtual int				SetMediaPlayerInitParam(MediaPlayerInitParam *pPlayerInitParam);

protected:

	void					functionAudioThread();

private:		

	int						createAudioRender(AYMediaAudioFormat &sAudioFormat);
	int						audio_render_frame(unsigned char*pData,unsigned int ulDataSize,unsigned int ulTimestamp);
	int						initAudioContext();
	int						unInitAudioContext();
	void					mixAudio(short*pDstBuffer,short*pRecordBuffer,short*pBackGroudBuffer,int bufferSize);
	int						mono2stereo(short*pDstBuffer,short*pSrcBuffer,int inSize);

private:
	//audio decode thread
	CAYMediaPlayerThread	*m_pAudioThread;

	//audio and video render
	IAudioRender			*m_pAudioRender;

	MediaPlayerInitParam    *m_pMediaPlayInitParam;


	AYMediaAudioFormat		m_sRecordAudioFormat;
	AYMediaAudioFormat		m_sBackGroudAudioFormat;

	void					*m_pJVM;
	void					*m_pViderRenderView;

	//pause flag 
	bool					m_bIsPauseFlag;
	bool					m_bIsNeedRender;
	bool					m_bAudioRunningFlag;


	ulu_CMutex				m_lookAudio;

	//file reader
	ISTDataReaderItf		*m_pRecodReader;
	ISTDataReaderItf		*m_pBackgroudReader;

	//audio volume
	int						m_iRecordAudioVolume;
	int						m_iBackGroudAudioVolume;

	//step size
	int						m_iRecordStepSize;
	int						m_iBackGroudStepSize;

};

#endif