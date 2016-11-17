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

CAYMediaPlayerThread::CAYMediaPlayerThread(CAYMediaPlayer*pMediaPlayer)
	: m_pMediaPlayer(pMediaPlayer)
{
	ut_begin();
}


CAYMediaPlayerThread::~CAYMediaPlayerThread()
{

}

void CAYMediaPlayerThread::ut_thread_function()
{
	m_pMediaPlayer->functionAudioThread();
}
CAYMediaPlayer::CAYMediaPlayer()
	: m_pAudioThread(NULL)
	, m_pMediaPlayInitParam(NULL)
	, m_pJVM(NULL)
	, m_pViderRenderView(NULL)
	, m_bIsPauseFlag(false)
	, m_bIsNeedRender(true)
	, m_pRecodReader(NULL)
	, m_pBackgroudReader(NULL)
	, m_iRecordStepSize(0)
	, m_iBackGroudStepSize(0)

{
	initAudioContext();
}

CAYMediaPlayer::~CAYMediaPlayer(){
	stop();
	unInitAudioContext();
}

int CAYMediaPlayer::initAudioContext(){

	m_pAudioRender = NULL;
	m_iRecordAudioVolume = 0;
	m_iBackGroudAudioVolume =0;
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
	m_pAudioThread = new CAYMediaPlayerThread(this);

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
	int iRet = 0;
	switch(uParamId){
	case PID_JAVA_VM:
		m_pJVM =pParam;
		if (m_pAudioRender)
		{
#ifdef _LINUX_ANDROID
			ulu_CAutoLock lock(&m_lookAudio);
			m_pAudioRender->SetNativeWindow(m_pJVM);
#endif
		}
		break;
	case PID_RECORD_FILE_PATH:
		{
			char*path=(char*)pParam;
			if (m_pRecodReader==NULL)
			{
				m_pRecodReader = (ISTDataReaderItf *)new STFileReader();
				m_pRecodReader->Open(path);
			}
		}
		break;

	case PID_BACKGROUD_FILE_PATH:
		{
			char*path=(char*)pParam;
			if (m_pBackgroudReader==NULL)
			{
				m_pBackgroudReader = (ISTDataReaderItf *)new STFileReader();
				m_pBackgroudReader->Open(path);
			}
		}
		break;

	case PID_AUDIO_RECORD_VOLUME:
		{
			ulu_CAutoLock lock(&m_lookAudio);
			m_iRecordAudioVolume = *(int*)pParam;
			if (m_pAudioRender)
			{
				m_pAudioRender->SetVolume(m_iRecordAudioVolume);
			}
		}
	case PID_AUDIO_BACKGROUD_VOLUME:
		{
			ulu_CAutoLock lock(&m_lookAudio);
			m_iBackGroudAudioVolume = *(int*)pParam;
			if (m_pAudioRender)
			{
				m_pAudioRender->SetVolume(m_iBackGroudAudioVolume);
			}
		}

	case PID_RECORDER_AUDIO_FORMAT:
		{
			AYMediaAudioFormat *pAudioFormat = (AYMediaAudioFormat*)pParam;
			m_sRecordAudioFormat.nChannels = pAudioFormat->nChannels;
			m_sRecordAudioFormat.nSamplesPerSec = pAudioFormat->nSamplesPerSec;
			m_sRecordAudioFormat.wBitsPerSample = pAudioFormat->wBitsPerSample;

			m_iRecordStepSize =(m_sRecordAudioFormat.nSamplesPerSec*m_sRecordAudioFormat.nChannels
				*m_sRecordAudioFormat.wBitsPerSample) / 50;
			break;
		}


	case PID_BACKGROUD_AUDIO_FORMAT:
		{
			AYMediaAudioFormat *pAudioFormat = (AYMediaAudioFormat*)pParam;
			m_sBackGroudAudioFormat.nChannels = pAudioFormat->nChannels;
			m_sBackGroudAudioFormat.nSamplesPerSec = pAudioFormat->nSamplesPerSec;
			m_sBackGroudAudioFormat.wBitsPerSample = pAudioFormat->wBitsPerSample;

			m_iBackGroudStepSize =(m_sBackGroudAudioFormat.nSamplesPerSec*m_sBackGroudAudioFormat.nChannels
				*m_sBackGroudAudioFormat.wBitsPerSample) / 50;
			break;
		}
	default:
		break;
	}

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
	unsigned char *recordBuffer = (unsigned char*)malloc(m_iRecordStepSize);
	unsigned char *backGroudBuffer =(unsigned char*)malloc(m_iBackGroudStepSize);
	unsigned int recoderReadPos=0;
	unsigned int backGroudReadPos=0;
	unsigned char*temp;
	while(m_bAudioRunningFlag){
		int read=0;
		if (m_pRecodReader!=NULL)
		{
			read=m_pRecodReader->Read(recordBuffer,recoderReadPos,m_iRecordStepSize);
			recoderReadPos+=read;
			//单声道转双声道
			if (m_sRecordAudioFormat.nChannels==1)
			{
				temp = (unsigned char*)malloc(read*2);
				mono2stereo((short*)temp,(short*)recordBuffer,read/2);
			}
		}
		if (m_pBackgroudReader!=NULL)
		{
			read=m_pRecodReader->Read(backGroudBuffer,backGroudReadPos,m_iBackGroudStepSize);
			backGroudReadPos+=read;
		}
		int mixSize =(m_iRecordStepSize*2>m_iBackGroudStepSize)?m_iBackGroudStepSize/2:m_iRecordStepSize;

		short*out =(short*)malloc(sizeof(short)*mixSize);
		mixAudio(out,(short*)temp,(short*)backGroudBuffer,mixSize);


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

	return iRetValue;

}
static inline int mulAdd(short in, short v, int a)
{
#if defined(__arm__) && !defined(__thumb__)
	STInt out;
	asm( "smlabb %[out], %[in], %[v], %[a] \n"
		: [out]"=r"(out)
		: [in]"%r"(in), [v]"r"(v), [a]"r"(a)
		: );
	return out;
#else
	return a + in * int(v);
#endif
}

static inline int mul(short in, short v)
{
#if defined(__arm__) && !defined(__thumb__)
	STInt out;
	asm( "smulbb %[out], %[in], %[v] \n"
		: [out]"=r"(out)
		: [in]"%r"(in), [v]"r"(v)
		: );
	return out;
#else
	return in * int(v);
#endif
}

static inline short clamp16(long sample)
{
	if ((sample>>15) ^ (sample>>31))
		sample = 0x7FFF ^ (sample>>31);
	return sample;
}


int CAYMediaPlayer::mono2stereo(short*pDstBuffer,short*pSrcBuffer,int inSize){    
	/*	unsigned short szBuf[4096];       
	unsigned short *pst = (unsigned short*)pData;    
	memset(szBuf, 0, sizeof(szBuf));    
	memcpy(szBuf, pData, nSize);  */  

	if (pDstBuffer==NULL||pSrcBuffer==NULL)
	{
		return -1;
	}
	for (int i = 0; i < inSize; i++)    
	{        
		pDstBuffer[i] = pSrcBuffer[i];
		pDstBuffer[i+1] = pSrcBuffer[i];

	}    
	return inSize * 2;

}


void CAYMediaPlayer::mixAudio(short*pDstBuffer,short*pRecordBuffer,short*pBackGroudBuffer,int bufferSize)
{
	short* pSrc1 = pRecordBuffer;
	short* pSrc2 = pBackGroudBuffer;
	int iChannels = m_sBackGroudAudioFormat.nChannels;
	int nProcessSample = bufferSize / iChannels;

	if (iChannels == 2)
	{
		long* pDstBuffer = (long*)pDstBuffer;
		while (nProcessSample--)
		{
			int nLeft0 = *pSrc1++;
			int nRight0 = *pSrc1++;
			nLeft0 = mul(nLeft0, m_iBackGroudAudioVolume);
			nRight0 = mul(nRight0, m_iBackGroudAudioVolume);
			int nLeft1 = *pSrc2++;
			int nRight1 = *pSrc2++;

			nLeft1 = mulAdd(nLeft1, m_iRecordAudioVolume, nLeft0) >> 12;
			nRight1 = mulAdd(nRight1, m_iRecordAudioVolume, nRight0) >> 12;

			nLeft1 = clamp16(nLeft1);
			nRight1 = clamp16(nRight1);

			*pDstBuffer++ = (nRight1 << 16) | (nLeft1 & 0xFFFF);
		}
	}
	else
	{
		short* pDstBuffer = (short*)pDstBuffer;
		while (nProcessSample--)
		{
			int nLeft0 = *pSrc1++;
			nLeft0 = mul(nLeft0, m_iRecordAudioVolume);

			int nLeft1 = *pSrc2++;
			nLeft1 = mulAdd(nLeft1, m_iRecordAudioVolume, nLeft0) >> 12;

			nLeft1 = clamp16(nLeft1);

			*pDstBuffer++ = (nLeft1 & 0xFFFF);
		}
	}
}


