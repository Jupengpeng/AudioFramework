#include "AYMediaPlayer.h"
#include "ulu_log.h"
#include <jni.h>
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
	m_bIsPauseFlag =false;
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

//#define DUMP_FILE
#ifdef DUMP_FILE
FILE *outFile = fopen("out.pcm","wb");
FILE *stereFile = fopen("stereo.pcm","wb");
#endif
void CAYMediaPlayer::functionAudioThread()
{
	void* pSourceFunc=NULL;
	void * pUserDate=NULL;
	unsigned char *recordBuffer = (unsigned char*)malloc(m_iRecordStepSize);
	unsigned char *backGroudBuffer =(unsigned char*)malloc(m_iBackGroudStepSize);
	unsigned int recoderReadPos=0;
	unsigned int backGroudReadPos=0;
	unsigned short*temp=NULL;
	while(m_bAudioRunningFlag){
		int read=0;
		int recordCout=0;
		int accoCount =0;
		if (m_pAudioRender==NULL)
		{
			int ret = createAudioRender(m_sBackGroudAudioFormat);
			if (ret!=0)
			{
				ulu_OS_Sleep(10);
				continue;
			}
		}
		//read from recorder
		if (m_pRecodReader!=NULL)
		{

			read=m_pRecodReader->Read(recordBuffer,recoderReadPos,m_iRecordStepSize);
			recoderReadPos+=read;
			if (read< 0)
			{
				break;
			}
			//单声道转双声道
			if (m_sRecordAudioFormat.nChannels==1)
			{
				int size =read/2;
				recordCout = size*2;
				temp = (unsigned short*)malloc(recordCout*sizeof(unsigned short));
				mono2stereo((short*)temp,(short*)recordBuffer,size);
#ifdef DUMP_FILE
				fwrite(temp,2,readCout,stereFile);
#endif
			}else{
				recordCout =read/2;
				temp =(unsigned short*)recordBuffer;
			}
		}


		// read from backgroud
		if (m_pBackgroudReader!=NULL)
		{
			read=m_pBackgroudReader->Read(backGroudBuffer,backGroudReadPos,m_iBackGroudStepSize);
			backGroudReadPos+=read;
			accoCount = read/2;
			if (read<0)
			{
				break;
			}

		}

		//mix audio
		int mixSize =(recordCout>accoCount)?accoCount:recordCout;
		mixSize = mixSize*2;
		char*out =(char*)malloc(mixSize);
		memset(out,0,mixSize);
		mixAudio(out,(char*)temp,(char*)backGroudBuffer,mixSize);
		ULULOGI("mix size =%d",mixSize);
		if (mixSize>0&&m_bIsNeedRender ==true)
		{
			audio_render_frame((unsigned char*)out,mixSize,0);
		}
#ifdef DUMP_FILE
		fwrite(out,1,mixSize,outFile);
#endif
		if (temp!=NULL)
		{
			free(temp);
		}
		if (out!=NULL)
		{
			free(out);
		}

	}

	if (recordBuffer!=NULL)
	{
		free(recordBuffer);
	}
	if (backGroudBuffer!=NULL)
	{
		free(backGroudBuffer);
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
	if (pDstBuffer==NULL||pSrcBuffer==NULL)
	{
		return -1;
	}
	int j=0;
	for (int i = 0; i < inSize; i++)    
	{        
		pDstBuffer[j] = pSrcBuffer[i];
		pDstBuffer[j+1] = pSrcBuffer[i];
		j+=2;

	}    
	return inSize * 2;

}

void  CAYMediaPlayer::mixAudio(char*pOutBuffer,char*pRecordBuffer,char*pBackGroudBuffer,int bufferSize)
{
	int i,j;
	for(i = 0; i < bufferSize; i+=2)
	{
		char tmp[2] = {0};
		short int nSrcValue = 0;
		short int nDestValue = 0;
		long nMidValue = 0;


		tmp[0] = pRecordBuffer[i];
		tmp[1] = pRecordBuffer[i+1];
		nSrcValue = *(int *)tmp;
		nMidValue += (long)nSrcValue;

		tmp[0] = pBackGroudBuffer[i];
		tmp[1] = pBackGroudBuffer[i+1];
		nSrcValue = *(int *)tmp;
		nMidValue += (long)nSrcValue;

		if(nMidValue > 32767)
		{
			nDestValue = 32767;
		}
		else if(nMidValue < -32768)
		{
			nDestValue = -32768;
		}
		else
		{
			nDestValue = nMidValue;
		}
		pOutBuffer[i] = (char)(nDestValue);
		pOutBuffer[i+1] = (char)(nDestValue>>8);
	}

}




//void CAYMediaPlayer::mixAudio(short*pOutBuffer,short*pRecordBuffer,short*pBackGroudBuffer,int bufferSize)
//{
//	short* pSrc1 = pRecordBuffer;
//	short* pSrc2 = pBackGroudBuffer;
//	int iChannels = m_sBackGroudAudioFormat.nChannels;
//	int nProcessSample = bufferSize / iChannels;
//	short *pOut = pOutBuffer;
//	while(bufferSize--){
//		short nLeft0 = *pSrc1++;
//		short nRight0 = *pSrc2++;
//		short out =nLeft0+nRight0;
//		*pOut++ =out;
//	}
//#if 0
//	if (iChannels == 1)
//	{
//		long* pDstBuffer = (long*)pOutBuffer;
//		while (nProcessSample--)
//		{
//			int nLeft0 = *pSrc1++;
//			int nRight0 = *pSrc1++;
//			nLeft0 = mul(nLeft0, m_iBackGroudAudioVolume);
//			nRight0 = mul(nRight0, m_iBackGroudAudioVolume);
//			int nLeft1 = *pSrc2++;
//			int nRight1 = *pSrc2++;
//
//			nLeft1 = mulAdd(nLeft1, m_iRecordAudioVolume, nLeft0) >> 12;
//			nRight1 = mulAdd(nRight1, m_iRecordAudioVolume, nRight0) >> 12;
//
//			nLeft1 = clamp16(nLeft1);
//			nRight1 = clamp16(nRight1);
//
//			*pDstBuffer++ = (nRight1 << 16) | (nLeft1 & 0xFFFF);
//		}
//	}
//	else
//	{
//		short* pDstBuffer = (short*)pOutBuffer;
//		while (nProcessSample--)
//		{
//			int nLeft0 = *pSrc1++;
//			nLeft0 = mul(nLeft0, m_iRecordAudioVolume);
//
//			int nLeft1 = *pSrc2++;
//			nLeft1 = mulAdd(nLeft1, m_iRecordAudioVolume, nLeft0) >> 12;
//
//			nLeft1 = clamp16(nLeft1);
//
//			*pDstBuffer++ = (nLeft1 & 0xFFFF);
//		}
//	}
//#endif
//}


