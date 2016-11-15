#include "AYMediaPlayer.h"
AYMEDIAPLAYER_SDK_API bool createMediaPlayerInstance(MediaPlayerInitParam * pInitParam, IAYMediaPlayer ** ppiMediaPlayer){

	CAYMediaPlayer *pMediaPlayer =  new CAYMediaPlayer();
	if(!pMediaPlayer)
		return false;
	pMediaPlayer->SetMediaPlayerInitParam(pInitParam);
	*ppiMediaPlayer = pMediaPlayer;
	return true;
}


AYMEDIAPLAYER_SDK_API void destroyMediaPlayerInstance(IAYMediaPlayer *& piMediaPlayer){
	delete piMediaPlayer;
}

CAYMediaPlayerThread::CAYMediaPlayerThread(CAYMediaPlayer*pMediaPlayer,AYMEDIAPLAYERTHREADTYPE eType)
	: m_pMediaPlayer(pMediaPlayer)
	, m_eType(eType)
{
	ut_begin();
}


CAYMediaPlayerThread::~CAYMediaPlayerThread()
{

}

void CAYMediaPlayerThread::ut_thread_function()
{
	if (m_pMediaPlayer!=NULL)
	{
		m_pMediaPlayer->functionAudioThread();
	}
}
CAYMediaPlayer::CAYMediaPlayer()
	: m_pAudioThread(NULL)
	, m_pMediaPlayInitParam(NULL)
	, m_pJVM(NULL)
	, m_pViderRenderView(NULL)
	, m_bIsPauseFlag(false)
	, m_bIsNeedRender(true)

{
	initAudioContext();
}

CAYMediaPlayer::~CAYMediaPlayer(){
	stop();
	unInitAudioContext();
}

int CAYMediaPlayer::initAudioContext(){

	m_pAudioRender = NULL;
	m_iAudioVolume = 0;
	return 0;
}

int CAYMediaPlayer::unInitAudioContext(){
	if (m_pAudioRender)
	{
		delete m_pAudioRender;
		m_pAudioRender =NULL;
	}
	return 0;
}



int CAYMediaPlayer::run()
{

	//audio thread start
	if (m_pAudioThread)
	{
		m_bAudioRunningFlag =false;
		delete m_pAudioThread;
	}
	m_bAudioRunningFlag = true;
	m_pAudioThread = new CAYMediaPlayerThread(this,AYMEDIAPLAYERTHREADTYPE_AUDIO);

	return 0;
}

int CAYMediaPlayer::pause()
{
	if (m_bIsPauseFlag == false)
	{
		m_bIsPauseFlag = true;
	}
	return 0;
}


int CAYMediaPlayer::resume()
{
	if (m_bIsPauseFlag == true)
	{
		m_bIsPauseFlag =false;
	}
	m_bIsNeedRender = true;
	return 0;
}

int CAYMediaPlayer::stop()
{
	//audio thread stop
	m_bAudioRunningFlag = false;
	if(m_pAudioThread){
		delete m_pAudioThread;
		m_pAudioThread = NULL;

	}
	return 0;
}

int CAYMediaPlayer::seekTo(unsigned int *uPosition)
{
	return 0;
}

int CAYMediaPlayer::getCurrentPosition(unsigned int * puPosition)
{
	return 0;
}

int CAYMediaPlayer::setParam(unsigned int uParamId, void * pParam)
{

	return 0;
}

int CAYMediaPlayer::getParam(unsigned int uParamId, void * pParam)
{

	return 0;
}




int CAYMediaPlayer::SetMediaPlayerInitParam(MediaPlayerInitParam *pPlayerInitParam)
{
	m_pMediaPlayInitParam =pPlayerInitParam;
	return 0;
}



void CAYMediaPlayer::functionAudioThread()
{
	void* pSourceFunc=NULL;
	void * pUserDate=NULL;
	int iRetValue= 0;
	unsigned char* pTempPcmData = NULL;
	unsigned int nPrePcmTimestamp = -1;
	unsigned int nPrePcmDuration = 0;
	while(m_bAudioRunningFlag){

	}

	unInitAudioContext();

}



int CAYMediaPlayer::audio_render_frame(unsigned char*pData,unsigned int ulDataSize,unsigned int ulTimestamp)
{

	ulu_CAutoLock lock(&m_lookAudio);

	int iRet = m_pAudioRender->Render(pData, ulDataSize);
	while(iRet == AUDIO_COMMIT_NEED_RETRY&&m_bAudioRunningFlag==1)
	{
		ulu_OS_Sleep(2);
		iRet = m_pAudioRender->Render(pData, ulDataSize);
	}
	return 0;
}



int CAYMediaPlayer::createAudioRender(AYMediaAudioFormat &sAudioFormat)
{
	ulu_CAutoLock lock(&m_lookAudio);
	int iRetValue = 0;
	if (m_pAudioRender !=NULL)
	{
		delete m_pAudioRender;
		m_pAudioRender =NULL;
	}
#ifdef __APPLE__
	iRetValue = CreateIOSAudioRender(m_pAudioRender);
#elif defined (_WINDOWS)
	iRetValue = CreateDXAudioRender(m_pAudioRender);
#elif defined (_LINUX_ANDROID)
	iRetValue = CreateAndroidAudioTrackRender(m_pAudioRender);
#endif // _DEBUG
	if (iRetValue == 1)
	{
#ifdef _LINUX_ANDROID
		if (m_pJVM ==NULL)
		{
			return -1;
		}
		m_pAudioRender->SetNativeWindow(m_pJVM);
#else
		if (m_pViderRenderView ==NULL)
		{
			return -1;
		}
		m_pAudioRender->SetNativeWindow(m_pViderRenderView);
#endif
		m_pAudioRender->SetAudioFormat(&sAudioFormat);
#ifdef _LINUX_ANDROID
		//m_nLatency = m_pAudioRender->GetLatency();
		//m_pAudioRender->Start();
		m_pAudioRender->SetVolume(100);
#elif defined(_WINDOWS)
		m_pAudioRender->SetVolume(100);
#else 

#endif
		m_pAudioRender->Start();
		iRetValue = 0;
	}
	//m_nStepSize = (m_sAudioFormat.nSamplesPerSec*m_sAudioFormat.nChannels*m_sAudioFormat.wBitsPerSample) / 50;

	return iRetValue;

}


