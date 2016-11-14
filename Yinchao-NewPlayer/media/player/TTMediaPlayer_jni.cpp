#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <jni.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/system_properties.h>
#include "TTLog.h"
#include "TTMediaPlayer.h"
#include "TTOsalConfig.h"
#include "TTFFT.h"
#include "TTJniEnvUtil.h"

//#define VERIFY_SIGNATURES

static const char kSignatures_md5[] = "\x85\x0d\x08\xd0\x2d\x18\xef\x52\xc5\x7c\x72\x68\x6d\xd1\xb6\xd8";
//    #include "signatures_md5.h"
//;

static const char kDebugSignatures_md5[] = "\x85\x0d\x08\xd0\x2d\x18\xef\x52\xc5\x7c\x72\x68\x6d\xd1\xb6\xd8";
//    #include "debug_signatures_md5.h"
//;

JavaVM* gJVM = NULL;
jobject g_Surface = NULL;

jint gViewWidth = 0;
jint gViewHeight = 0;

TTBool gAudioEffectLowDelay = ETTTrue;

static const char* const kClassPathName = "com/android/ychao/media/player/YCMediaPlayer";
static const char* const kClassFieldName = "mNativePlayerPara";

extern void ConfigProxyServer(TTUint32 aIP, TTInt Port, const TTChar* aAuthen, TTBool aUseProxy);

// ref-counted object for callbacks
class JNITTMediaPlayerListener: public ITTMediaPlayerObserver
{
public:
	JNITTMediaPlayerListener(jobject thiz, jobject weak_thiz, jmethodID postEvnt, JNIEnv* aEnv);
	~JNITTMediaPlayerListener();
	void PlayerNotifyEvent(TTNotifyMsg aMsg, TTInt aArg1, TTInt aArg2, const TTChar* aArg3);

private:
	JNITTMediaPlayerListener();
	jclass      	mClass;     // Reference to MediaPlayer class
	jobject     	mObject;    // Weak ref to MediaPlayer Java object to call on
	JNIEnv*			mEnv;
	jmethodID	    post_event;
};

JNITTMediaPlayerListener::JNITTMediaPlayerListener(jobject thiz, jobject weak_thiz, jmethodID postEvnt, JNIEnv* aEnv)
{
	// Hold onto the MediaPlayer class for use in calling the static method
	// that posts events to the application thread.
	mEnv = aEnv;
	post_event = postEvnt;
	jclass clazz = mEnv->GetObjectClass(thiz);
	if (clazz == NULL) {
		LOGE("Can't create JNITTMediaPlayerListener");
		mEnv->ThrowNew(clazz, "Can't create JNITTMediaPlayerListener");
		return;
	}
	mClass = (jclass)mEnv->NewGlobalRef(clazz);

	// We use a weak reference so the MediaPlayer object can be garbage collected.
	// The reference is only used as a proxy for callbacks.
	mObject = mEnv->NewGlobalRef(weak_thiz);
}

JNITTMediaPlayerListener::~JNITTMediaPlayerListener()
{
	// remove global references
	CJniEnvUtil	env(gJVM);
	JNIEnv* penv = env.getEnv();
	if (penv!=NULL){
		penv->DeleteGlobalRef(mObject);
		penv->DeleteGlobalRef(mClass);
	}
}

void JNITTMediaPlayerListener::PlayerNotifyEvent(TTNotifyMsg aMsg, TTInt aArg1, TTInt aArg2, const TTChar* aArg3)
{
	CJniEnvUtil	env(gJVM);
	JNIEnv* penv = env.getEnv();
	if (penv)
	{
		jstring jString = (aArg3 != NULL) ? penv->NewStringUTF(aArg3) : NULL;
		penv->CallStaticVoidMethod(mClass, post_event, mObject, aMsg, aArg1, aArg2, jString);
		LOGI("PlayerNotifyEvent to java, msgID = %d", (int)aMsg);
		/* delete local reference */
		if(jString)
			penv->DeleteLocalRef(jString);
	}
}

typedef struct MediaPara
{
	//jmethodID			 post_event;
	ITTMediaPlayer* 	 iMediaPlayer;
	RTTCritical			 iCriticalVisual;

#define FreqWaveValid
#ifdef	FreqWaveValid
	TTInt16*			 iFreq;
	TTInt16*			 iWave;
#endif

	JNIEnv*						iCmdEnvPtr;
	JNITTMediaPlayerListener*	iPlayerListener;

	MediaPara()
	{
#ifdef	FreqWaveValid
		iFreq = NULL;
		iWave = NULL;
#endif
		iMediaPlayer = NULL;
		iCmdEnvPtr  = NULL;
		iCriticalVisual.Create();
		iPlayerListener = NULL;
	}

	~MediaPara()
	{
		iCriticalVisual.Destroy();
		SAFE_DELETE(iPlayerListener);
#ifdef	FreqWaveValid
		SAFE_DELETE_ARRAY(iFreq);
		SAFE_DELETE_ARRAY(iWave);
#endif

	}

} MediaPara ;


void NotifyDecThreadAttach()
{
	//gJVM->AttachCurrentThread(&gDecThreadEnvPtr, NULL);
}

void NotifyDecThreadDetach()
{
	gJVM->DetachCurrentThread();
}


//static void ychao_media_YCMediaPlayer_native_init(JNIEnv* env)
//{
//}

static void SetSurface(JNIEnv* env,jobject obj, MediaPara* pMediaPara)
{
	jclass		clazz = env->FindClass(kClassPathName);
	if (clazz == NULL) return;

	jfieldID	surfaceID = env->GetFieldID(clazz, "mSurface", "Landroid/view/Surface;");
	if (surfaceID == NULL) return;

	jobject		surfaceObj = env->GetObjectField(obj, surfaceID);

	if(g_Surface != NULL)
	{
		env->DeleteGlobalRef(g_Surface);
		g_Surface = NULL;
	}


	g_Surface = env->NewGlobalRef(surfaceObj);
	if(pMediaPara)
	{
		pMediaPara->iMediaPlayer->SetView(g_Surface);
	}

	env->DeleteLocalRef(clazz);

	return;
}

static void GetScreenSize(JNIEnv* env,jobject obj)
{
	jclass		clazz = env->FindClass(kClassPathName);
	if (clazz == NULL) return;

	jfieldID widthID = env->GetFieldID(clazz, "mViewWidth", "I");
	if (widthID == NULL) return;

	gViewWidth = env->GetIntField(obj, widthID);

	jfieldID heightID = env->GetFieldID(clazz, "mViewHeight", "I");
	if (heightID == NULL) return;

	gViewHeight = env->GetIntField(obj, heightID);

	env->DeleteLocalRef(clazz);

	return;
}

static void ychao_media_YCMediaPlayer_native_setup(JNIEnv* env, jobject thiz, jobject weak_this, jbyteArray signatures, jint samplerate, jstring plugin_path) {
	LOGI("mediaplayer native_setup");

	MediaPara* pMediaPara = new MediaPara;
	pMediaPara->iCmdEnvPtr = env;
	env->GetJavaVM(&gJVM);

	jclass className = env->FindClass(kClassPathName);
	jmethodID postEvent = env->GetStaticMethodID(className, "postEventFromNative", "(Ljava/lang/Object;IIILjava/lang/Object;)V");

#ifdef VERIFY_SIGNATURES
	jbyte* signatureBytes = env->GetByteArrayElements(signatures, JNI_FALSE);
    jsize byteLength = env->GetArrayLength(signatures);
    if (memcmp(signatureBytes, kSignatures_md5, byteLength) == 0
        || memcmp(signatureBytes, kDebugSignatures_md5, byteLength) == 0) {
#endif
	const char *pPluginChars = env->GetStringUTFChars(plugin_path, NULL);
	JNITTMediaPlayerListener* PlayerListener = new JNITTMediaPlayerListener(thiz, weak_this, postEvent, env);
	ITTMediaPlayer* pMediaPlayer = new CTTMediaPlayer(PlayerListener, pPluginChars);

	pMediaPara->iPlayerListener = PlayerListener;
	pMediaPara->iMediaPlayer = pMediaPlayer;
	//audioTrack_native_init
	jclass AudioTrack = env->FindClass("com/android/ychao/media/player/YCAudioTrack");
	jclass* pAudioTrack = new jclass((jclass)env->NewGlobalRef(AudioTrack));
	pMediaPlayer->SaveAudioTrackJClass(pAudioTrack);
	pMediaPlayer->SetMaxOutPutSamplerate(samplerate);

	TTChar szProp[64];
	memset (szProp, 0, 64);
	//__system_property_get ("ro.build.version.release", szProp);
	if (strstr (szProp, "2.") == szProp || strstr (szProp, "1.") == szProp) {
		//jclass VideoTrack = env->FindClass("com/sds/android/ttpod/media/player/TTVideoTrack");
		//jclass* pVideoTrack = new jclass((jclass)env->NewGlobalRef(VideoTrack));
		//pMediaPlayer->SaveVideoTrackJClass(pVideoTrack);
		//env->DeleteLocalRef(VideoTrack);
	}

	jfieldID MediaPlayPara = env->GetFieldID(className,kClassFieldName, "J");
	env->SetLongField(thiz,MediaPlayPara, (long)pMediaPara);
	env->DeleteLocalRef(className);
	env->ReleaseStringUTFChars(plugin_path, pPluginChars);
	env->DeleteLocalRef(AudioTrack);

	//audioTrack_native_init
#ifdef VERIFY_SIGNATURES
	}
    env->ReleaseByteArrayElements(signatures, signatureBytes, 0);
#endif

	if (pMediaPlayer == NULL) {
		LOGI("Create Player Error");
	} else {
		pMediaPara->iFreq = new TTInt16[KMaxWaveSample];
		pMediaPara->iWave = new TTInt16[KMaxWaveSample * 2];
	}
}

static void ychao_media_YCMediaPlayer_native_release(JNIEnv* env, jobject thiz, jlong context)
{
	LOGI("mediaplayer native_release");
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return;
	}

	jclass* pAudiotrackJclass = NULL;
	jclass* pVideotrackJclass = NULL;
	if (pMediaPara->iMediaPlayer != NULL)
	{
		pAudiotrackJclass = static_cast<jclass*>(pMediaPara->iMediaPlayer->GetAudiotrackClass());
		pVideotrackJclass = static_cast<jclass*>(pMediaPara->iMediaPlayer->GetVideotrackClass());
		pMediaPara->iMediaPlayer->Stop(true);
		pMediaPara->iMediaPlayer->Release();
		LOGI("mediaplayer native_release 1");
		pMediaPara->iMediaPlayer = NULL;
	}
	SAFE_DELETE(pMediaPara);

	if(pAudiotrackJclass !=NULL)
	{
		env->DeleteGlobalRef(*pAudiotrackJclass);
		delete pAudiotrackJclass;
	}

	if(pVideotrackJclass !=NULL)
	{
		env->DeleteGlobalRef(*pVideotrackJclass);
		delete pVideotrackJclass;
	}

	if(g_Surface != NULL)
	{
		env->DeleteGlobalRef(g_Surface);
		g_Surface = NULL;
	}

	LOGI("native_release Finish");
}

static void ychao_media_YCMediaPlayer_setCacheFilePath_native(JNIEnv* env, jobject thiz, jlong context, jstring uri)
{
	LOGI("setCacheFilePath start");
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return;
	}

	if (pMediaPara->iMediaPlayer != NULL)
	{
		const char *uriStr = env->GetStringUTFChars(uri, NULL);
		pMediaPara->iMediaPlayer->SetCacheFilePath(uriStr);
		env->ReleaseStringUTFChars(uri, uriStr);

	}
	LOGI("setCacheFilePath End");
}

static int ychao_media_YCMediaPlayer_setDataSourceSync_native(JNIEnv* env, jobject thiz, jlong context, jstring uri, jint flag)
{
	LOGI("native_setDataSourceSync");
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return  -1;
	}

	GetScreenSize(env, thiz);
	SetSurface(env, thiz, pMediaPara);

	int nErr = -1;
	if (pMediaPara->iMediaPlayer != NULL)
	{
		const char *uriStr = env->GetStringUTFChars(uri, NULL);
		nErr = pMediaPara->iMediaPlayer->SetDataSourceSync(uriStr, flag);
		env->ReleaseStringUTFChars(uri, uriStr);
	}
	LOGI("native_setDataSourceSync return %d", nErr);

	return nErr;
}

static int ychao_media_YCMediaPlayer_setDataSourceAsync_native(JNIEnv* env, jobject thiz, jlong context, jstring uri, jint flag)
{
	LOGI("native_setDataSourceAsync");
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return -1;
	}
	LOGI("native_setDataSourceAsync");
//	if(!(flag&0xA)) {
//		GetScreenSize(env, thiz);
//		SetSurface(env, thiz, pMediaPara);
//	}
	int nErr = -1;
	LOGI("native_setDataSourceAsync");
	if (pMediaPara->iMediaPlayer != NULL)
	{
		LOGI("native_setDataSourceAsync");
		const char *uriStr = env->GetStringUTFChars(uri, NULL);
		LOGI("native_setDataSourceAsync");
		nErr = pMediaPara->iMediaPlayer->SetDataSourceAsync(uriStr, (int)flag);
		LOGI("native_setDataSourceAsync");
		env->ReleaseStringUTFChars(uri, uriStr);
	}
	LOGI("native_setDataSourceAsync");
	return nErr;
}

static int ychao_media_YCMediaPlayer_play_native(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return TTKErrNotFound;
	}

	if (pMediaPara->iMediaPlayer != NULL)
	{
#ifdef FreqWaveValid
		pMediaPara->iCriticalVisual.Lock();
		memset(pMediaPara->iFreq, 0, KMaxWaveSample * sizeof(TTInt16));
		pMediaPara->iCriticalVisual.UnLock();
#endif

		return pMediaPara->iMediaPlayer->Play();
	}
	else
	{
		LOGE("JNI-Play gMediaPlayer == NULL");
		return TTKErrNotFound;
	}
}

static void ychao_media_YCMediaPlayer_pause_native(JNIEnv* env, jobject thiz, jlong context, jboolean afadeout)
{
	LOGI("ITTMediaPlayer pause");
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return;
	}

	if (pMediaPara->iMediaPlayer != NULL)
	{
		pMediaPara->iMediaPlayer->Pause(afadeout);
		LOGI("ITTMediaPlayer pause ok");
	}
	else
	{
		LOGI("Player Not Existed");
	}
}

static void ychao_media_YCMediaPlayer_resume_native(JNIEnv* env, jobject thiz, jlong context, jboolean afadein)
{
	LOGI("MediaPlayer resume");
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return;
	}

	if (pMediaPara->iMediaPlayer != NULL)
	{
		pMediaPara->iMediaPlayer->Resume(afadein);
		LOGI("MediaPlayer resume ok");
	}
	else
	{
		LOGI("Player Not Existed");
	}
}

static void ychao_media_YCMediaPlayer_setAudioEffectLowDelay_native(JNIEnv* env, jobject thiz, jlong context, jboolean lowdelay)
{
	gAudioEffectLowDelay = lowdelay;
}

static void ychao_media_YCMediaPlayer_setvolume_native(JNIEnv* env, jobject thiz, jlong context, jint lvolume, jint rvolume)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return;
	}

	if (pMediaPara->iMediaPlayer != NULL)
	{
		pMediaPara->iCriticalVisual.Lock();
		pMediaPara->iMediaPlayer->SetVolume(lvolume, rvolume);
		pMediaPara->iCriticalVisual.UnLock();
	}
	else
	{
		LOGI("Player Not Existed");
	}
}

static int ychao_media_YCMediaPlayer_stop_native(JNIEnv* env, jobject thiz, jlong context)
{
	LOGI("MediaPlayer stop");
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return -1;
	}

	int nErr = -1;
	if (pMediaPara->iMediaPlayer != NULL)
	{
		nErr = pMediaPara->iMediaPlayer->Stop();
	}
	else
	{
		LOGI("Player Not Existed");
	}

	LOGI("MediaPlayer stop OK");
	return nErr;
}

static int ychao_media_YCMediaPlayer_setSurface_native(JNIEnv* env, jobject thiz, jlong context)
{
	LOGI("MediaPlayer setSurface");
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return -1;
	}

	SetSurface(env, thiz, pMediaPara);
	GetScreenSize(env, thiz);

	LOGI("MediaPlayer setSurface OK");
	return 0;
}

static void ychao_media_YCMediaPlayer_setPlayRange_native(JNIEnv* env, jobject thiz, jlong context, jint start, jint end)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return;
	}

	if (pMediaPara->iMediaPlayer != NULL)
	{
		pMediaPara->iMediaPlayer->SetPlayRange((unsigned int)start, (unsigned int)end);
		LOGI("MediaPlayer SetPlayRange [%d, %d] ok", start, end);
	}
	else
	{
		LOGI("Player Not Existed");
	}

}

static int ychao_media_YCMediaPlayer_setPosition_native(JNIEnv* env, jobject thiz, jlong context, jint pos, jint flag)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	int nPos = 0;
	if (pMediaPara == NULL)
	{
		return -1;
	}

	if (pos < 0)
	{
		LOGE("Seek pos < 0");
		return pos;
	}

	LOGI("MediaPlayer setposition %d", pos);
	if (pMediaPara->iMediaPlayer != NULL)
	{
		nPos = pMediaPara->iMediaPlayer->SetPosition((TTInt64)pos, (int)flag);
		LOGI("MediaPlayer setposition %d ok", pos);
	}
	else
	{
		LOGI("Player Not Existed");
	}

	return nPos;
}

static void ychao_media_YCMediaPlayer_setActiveNetWorkType_native(JNIEnv* env, jobject thiz, jlong context, jint type)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return;
	}

	if (pMediaPara->iMediaPlayer != NULL)
	{
		pMediaPara->iMediaPlayer->SetActiveNetWorkType((TTActiveNetWorkType)type);
	}
	else
	{
		LOGI("Player Not Existed");
	}
}

static void ychao_media_YCMediaPlayer_setDecoderType_native(JNIEnv* env, jobject thiz, jlong context, jint type)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return;
	}

	if (pMediaPara->iMediaPlayer != NULL)
	{
		pMediaPara->iMediaPlayer->SetDecoderType((TTDecoderType)type);
	}
	else
	{
		LOGI("Player Not Existed");
	}
}

static int ychao_media_YCMediaPlayer_getPosition_native(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return 0;
	}

	int pos = 0;
	if (pMediaPara->iMediaPlayer != NULL)
	{
		pMediaPara->iCriticalVisual.Lock();
		pos = (int)(pMediaPara->iMediaPlayer->GetPosition());
		pMediaPara->iCriticalVisual.UnLock();
	}
	else
	{
		LOGI("Player Not Existed");
	}

	return pos;
}

static int ychao_media_YCMediaPlayer_duration_native(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return 0;
	}

	int duration = 0;
	if(pMediaPara->iMediaPlayer != NULL)
	{
		duration = pMediaPara->iMediaPlayer->Duration();
	}
	else
	{
		LOGI("Player Not Existed");
	}

	return duration;
}

static int ychao_media_YCMediaPlayer_size_native(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return 0;
	}

	int size = 0;
	if(pMediaPara->iMediaPlayer != NULL)
	{
		size = pMediaPara->iMediaPlayer->Size();
	}
	else
	{
		LOGI("Player Not Existed");
	}

	return size;
}

static int ychao_media_YCMediaPlayer_bufferedSize_native(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return 0;
	}

	int bufferedSize = 0;
	if(pMediaPara->iMediaPlayer != NULL)
	{
		bufferedSize = pMediaPara->iMediaPlayer->BufferedSize();
	}
	else
	{
		LOGI("Player Not Existed");
	}

	return bufferedSize;
}

static int ychao_media_YCMediaPlayer_bufferedPercent_native(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return TTKErrNotSupported;
	}

	int nBufferedPercent = 0;
	int nErr = TTKErrNotSupported;
	if((pMediaPara->iMediaPlayer != NULL) && ((nErr = pMediaPara->iMediaPlayer->BufferedPercent(nBufferedPercent)) == TTKErrNone))
	{
		return nBufferedPercent;
	}

	return nErr;
}

static int ychao_media_YCMediaPlayer_bufferBandWidth_native(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return 0;
	}

	int nBandWidth = 0;
	if(pMediaPara->iMediaPlayer != NULL)
	{
		nBandWidth = pMediaPara->iMediaPlayer->BandWidth();
	}

	return nBandWidth;
}

static int ychao_media_YCMediaPlayer_bufferBandPercent_native(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return 0;
	}

	int nBandPercent = 0;
	if(pMediaPara->iMediaPlayer != NULL)
	{
		nBandPercent = pMediaPara->iMediaPlayer->BandPercent();
	}

	return nBandPercent;
}

static int ychao_media_YCMediaPlayer_getStatus_native(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return -1;
	}

	int playstatus = -1;
	if (pMediaPara->iMediaPlayer != NULL)
	{
		playstatus = (int)(pMediaPara->iMediaPlayer->GetPlayStatus());
	}
	else
	{
		//LOGI("Player Not Existed");
	}

	return playstatus;
}

static int ychao_media_YCMediaPlayer_getCurFreqAndWave_native(JNIEnv* env, jobject thiz, jlong context, jshortArray freqarr, jshortArray wavearr, jint samplenum)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
		return -1;

	int nErr = -1;
#ifdef FreqWaveValid
	if (pMediaPara->iMediaPlayer != NULL)
	{
		if (pMediaPara->iMediaPlayer->GetPlayStatus() == EStatusPlaying)
		{
			pMediaPara->iCriticalVisual.Lock();
			nErr = pMediaPara->iMediaPlayer->GetCurrentFreqAndWave(pMediaPara->iFreq, pMediaPara->iWave, samplenum);
			pMediaPara->iCriticalVisual.UnLock();

			if (TTKErrNone == nErr)
			{
				env->SetShortArrayRegion(freqarr, 0, (jsize)(samplenum), (jshort*)(pMediaPara->iFreq));
				env->SetShortArrayRegion(wavearr, 0, (jsize)(samplenum), (jshort*)(pMediaPara->iWave));
			}
		}
	}
	else
	{
		LOGI("Player Not Existed");
	}
#endif
	return nErr;
}

static int ychao_media_YCMediaPlayer_getCurFreq_native(JNIEnv* env, jobject thiz, jlong context, jshortArray freqarr, jint freqnum)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return -1;
	}

	int nErr = -1;
#ifdef FreqWaveValid
	if (pMediaPara->iMediaPlayer != NULL)
	{
		if (pMediaPara->iMediaPlayer->GetPlayStatus() == EStatusPlaying)
		{
			if((pMediaPara->iWave == NULL) || (pMediaPara->iFreq == NULL))
			{
				LOGI("Wave Freq Invalid");
			}

			pMediaPara->iCriticalVisual.Lock();
			nErr = pMediaPara->iMediaPlayer->GetCurrentFreqAndWave(pMediaPara->iFreq, pMediaPara->iWave, freqnum);
			pMediaPara->iCriticalVisual.UnLock();

			if (TTKErrNone == nErr)
			{
				env->SetShortArrayRegion(freqarr, 0, (jsize)(freqnum), (jshort*)(pMediaPara->iFreq));
			}
			else
			{
				LOGE("GetFreqErr");
			}
		}
	}
	else
	{
		LOGI("Player Not Existed");
	}
#endif

	return nErr;
}

static int ychao_media_YCMediaPlayer_getCurWave_native(JNIEnv* env, jobject thiz, jlong context, jshortArray wavearr, jint wavenum)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL)
	{
		return -1;
	}

	int nErr = -1;
#ifdef FreqWaveValid
	if (pMediaPara->iMediaPlayer != NULL)
	{
		if (pMediaPara->iMediaPlayer->GetPlayStatus() == EStatusPlaying)
		{
			if((pMediaPara->iWave == NULL) || (pMediaPara->iFreq == NULL))
			{
				LOGI("Wave Freq Invalid");
			}

			pMediaPara->iCriticalVisual.Lock();
			nErr = pMediaPara->iMediaPlayer->GetCurrentFreqAndWave(NULL, pMediaPara->iWave, wavenum);
			pMediaPara->iCriticalVisual.UnLock();

			if (TTKErrNone == nErr)
			{
				LOGI("GetwaveOK");
				env->SetShortArrayRegion(wavearr, 0, (jsize)(wavenum), (jshort*)(pMediaPara->iWave));
			}
		}
	}
	else
	{
		LOGI("Player Not Existed");
	}
#endif

	return nErr;
}


static void ychao_media_YCMediaPlayer_CongfigProxyServer_native(JNIEnv* env, jobject thiz, jlong context, jstring aIP, jint aPort, jstring authenkey, jboolean aUserProxy)
{
	struct in_addr address;
	const char *pAuthenkey = env->GetStringUTFChars(authenkey, NULL);
	const char *pIP = env->GetStringUTFChars(aIP, NULL);
	TTBool bUserProxy = (TTBool)aUserProxy;
	inet_aton(pIP, &address);
	ConfigProxyServer(address.s_addr, aPort, pAuthenkey, bUserProxy);
	env->ReleaseStringUTFChars(authenkey, pAuthenkey);
	env->ReleaseStringUTFChars(aIP, pIP);

	LOGI("Player set networkProxy aUserProxy %d", (int)aUserProxy);
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara != NULL && pMediaPara->iMediaPlayer) {
		pMediaPara->iMediaPlayer->SetNetWorkProxy(bUserProxy);
	}
}

//static void ychao_media_YCMediaPlayer_ClearScreen_native(JNIEnv* env, jobject thiz, jlong context, jint bClear)
//{
//	MediaPara* pMediaPara = (MediaPara* )context;
//	if (pMediaPara == NULL)
//	{
//		return;
//	}
//
//	int nBandWidth = 0;
//	if(pMediaPara->iMediaPlayer != NULL)
//	{
//		TTBool aClear = (TTBool)bClear;
//		pMediaPara->iMediaPlayer->ClearScreen(aClear);
//	}
//
//	return;
//}


static int ychao_media_YCMediaPlayer_getPCMData(JNIEnv* env, jobject thiz, jlong context, jstring uri, jbyteArray aArray)
{
	LOGI("getPCMData start");
	int nErr = -1;

	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL || pMediaPara->iMediaPlayer == NULL)
	{
		return -1;
	}

	int playstatus = -1;

	int nArraySize = env->GetArrayLength(aArray);

	const char *uriStr = env->GetStringUTFChars(uri, NULL);
	nErr = pMediaPara->iMediaPlayer->Decode(uriStr, nArraySize);
	env->ReleaseStringUTFChars(uri, uriStr);

	if (nErr == TTKErrNone)
	{
		TTUint8* pData = pMediaPara->iMediaPlayer->GetPCMData();
		if (pData)
			env->SetByteArrayRegion(aArray, 0, (jsize)(pMediaPara->iMediaPlayer->GetPCMDataSize()), (jbyte*)pData);
	}

	return nErr;
}

static int ychao_media_YCMediaPlayer_getPcmChannel(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL || pMediaPara->iMediaPlayer == NULL)
	{
		return -1;
	}
	return pMediaPara->iMediaPlayer->GetPCMDataChannle();
}

static int ychao_media_YCMediaPlayer_getPcmSamplerate(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL || pMediaPara->iMediaPlayer == NULL)
	{
		return -1;
	}
	return pMediaPara->iMediaPlayer->GetPCMDataSamplerate();
}

static int ychao_media_YCMediaPlayer_getPcmSize(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL || pMediaPara->iMediaPlayer == NULL)
	{
		return -1;
	}
	return pMediaPara->iMediaPlayer->GetPCMDataSize();
}

static void ychao_media_YCMediaPlayer_cancelPcm(JNIEnv* env, jobject thiz, jlong context)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL || pMediaPara->iMediaPlayer == NULL)
	{
	}
	else
		pMediaPara->iMediaPlayer->CancelGetPCM();
}

static void ychao_media_YCMediaPlayer_setPcmRange(JNIEnv* env, jobject thiz, jlong context, jint start, jint end, jint decodeStartPos)
{
	MediaPara* pMediaPara = (MediaPara* )context;
	if (pMediaPara == NULL || pMediaPara->iMediaPlayer == NULL)
	{
	}
	pMediaPara->iMediaPlayer->SetTimeRange((unsigned int)start, (unsigned int)end, (unsigned int)decodeStartPos);
}

static JNINativeMethod gMethods[] = {
		//{"nativeInit",         "()V",            					(void *)ychao_media_YCMediaPlayer_native_init},
		{"nativeSetup",        "(Ljava/lang/Object;[BILjava/lang/String;)V",    			(void *)ychao_media_YCMediaPlayer_native_setup},
		{"nativeRelease",      "(J)V",            					(void *)ychao_media_YCMediaPlayer_native_release},
		{"nativePlay",      			"(J)I",       						(void *)ychao_media_YCMediaPlayer_play_native},
		{"nativeSetDataSourceAsync", 	"(JLjava/lang/String;I)I",    (void *)ychao_media_YCMediaPlayer_setDataSourceAsync_native},
		{"nativeSetDataSourceSync", 	"(JLjava/lang/String;I)I",    (void *)ychao_media_YCMediaPlayer_setDataSourceSync_native},
		{"nativeSetCacheFilePath", 	"(JLjava/lang/String;)V",    (void *)ychao_media_YCMediaPlayer_setCacheFilePath_native},
		{"nativeStop",      	"(J)I",       						(void *)ychao_media_YCMediaPlayer_stop_native},
		{"nativeSetSurface",      		"(J)I",       				(void *)ychao_media_YCMediaPlayer_setSurface_native},
		{"nativePause",      			"(JZ)V",       						(void *)ychao_media_YCMediaPlayer_pause_native},
		{"nativeResume",      		"(JZ)V",       						(void *)ychao_media_YCMediaPlayer_resume_native},
		{"nativeSetPosition",      	"(JII)I",       						(void *)ychao_media_YCMediaPlayer_setPosition_native},
		{"nativeSetPlayRange",      	"(JII)V",       						(void *)ychao_media_YCMediaPlayer_setPlayRange_native},
		{"nativeGetPosition",      	"(J)I",       						(void *)ychao_media_YCMediaPlayer_getPosition_native},
		{"nativeSetActiveNetWorkType", "(JI)V",       						(void *)ychao_media_YCMediaPlayer_setActiveNetWorkType_native},
		{"nativeSetDecoderType", "(JI)V",       						(void *)ychao_media_YCMediaPlayer_setDecoderType_native},
		{"nativeDuration",      		"(J)I",       						(void *)ychao_media_YCMediaPlayer_duration_native},
		{"nativeSize",      		"(J)I",       						(void *)ychao_media_YCMediaPlayer_size_native},
		{"nativeBufferedSize",      		"(J)I",       						(void *)ychao_media_YCMediaPlayer_bufferedSize_native},
		{"nativeBufferedPercent",   	"(J)I",       				   		(void *)ychao_media_YCMediaPlayer_bufferedPercent_native},
		{"nativeBufferBandWidth",   	"(J)I",       				   		(void *)ychao_media_YCMediaPlayer_bufferBandWidth_native},
		{"nativeGetStatus",      		"(J)I",       						(void *)ychao_media_YCMediaPlayer_getStatus_native},
		{"nativeGetCurFreqAndWave",   "(J[S[SI)I",       			(void *)ychao_media_YCMediaPlayer_getCurFreqAndWave_native},
		{"nativeGetCurWave",   		"(J[SI)I",       				(void *)ychao_media_YCMediaPlayer_getCurWave_native},
		{"nativeGetCurFreq",   		"(J[SI)I",       				(void *)ychao_media_YCMediaPlayer_getCurFreq_native},
		{"nativeSetVolume",   		"(JII)V",       				   	(void *)ychao_media_YCMediaPlayer_setvolume_native},
		{"nativeSetAudioEffectLowDelay" ,  "(JZ)V",                  (void *)ychao_media_YCMediaPlayer_setAudioEffectLowDelay_native},
		{"nativeCongfigProxyServer" ,  "(JLjava/lang/String;ILjava/lang/String;Z)V",  (void *)ychao_media_YCMediaPlayer_CongfigProxyServer_native},
		{"nativeBufferBandPercent",   	"(J)I",       				   		(void *)ychao_media_YCMediaPlayer_bufferBandPercent_native},
		//{"nativeClearScrren", "(II)V",       						(void *)ychao_media_YCMediaPlayer_ClearScreen_native},
		//{"nativeGetPCMData", 	    "(ILjava/lang/String;[B)I",        (void *)ychao_media_YCMediaPlayer_getPCMData},
		//{"nativeGetPcmSize",      		"(I)I",       				(void *)ychao_media_YCMediaPlayer_getPcmSize},
		//{"nativeGetPcmChannel",      "(I)I",       						(void *)ychao_media_YCMediaPlayer_getPcmChannel},
		//{"nativeGetPcmSamplerate",    "(I)I",       						(void *)ychao_media_YCMediaPlayer_getPcmSamplerate},
		//{"nativeCancelPcm",      	  "(I)V",       						(void *)ychao_media_YCMediaPlayer_cancelPcm},
		//{"nativeSetPcmRange",      	"(IIII)V",       					(void *)ychao_media_YCMediaPlayer_setPcmRange},
};

// ----------------------------------------------------------------------------

// This function only registers the native methods
static int register_ttpod_media_MediaPlayer(JNIEnv *env)
{
	jclass className = env->FindClass(kClassPathName);
	if (className == NULL) {
		LOGE("Can't find %s\n", kClassPathName);
		return -1;
	}

	if (env->RegisterNatives(className, gMethods, sizeof(gMethods)
												  / sizeof(gMethods[0])) != JNI_OK) {
		LOGE("ERROR: Register mediaplayer jni methods failed\n");
		env->DeleteLocalRef(className);
		return -1;
	}

	env->DeleteLocalRef(className);
	LOGI("register %s succeed\n", kClassPathName);
	return 0;
}

//extern int register_ttpod_media_AudioEffect(JNIEnv *env);

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;
	LOGI("MediaPlayer: JNI OnLoad\n");
#ifdef JNI_VERSION_1_6
	if (result==-1 && vm->GetEnv((void**) &env, JNI_VERSION_1_6) == JNI_OK) {
		LOGI("JNI_OnLoad: JNI_VERSION_1_6\n");
		result = JNI_VERSION_1_6;
	}
#endif
#ifdef JNI_VERSION_1_4
	if (result==-1 && vm->GetEnv((void**) &env, JNI_VERSION_1_4) == JNI_OK) {
		LOGI("JNI_OnLoad: JNI_VERSION_1_4\n");
		result = JNI_VERSION_1_4;
	}
#endif
#ifdef JNI_VERSION_1_2
	if (result==-1 && vm->GetEnv((void**) &env, JNI_VERSION_1_2) == JNI_OK) {
		LOGI("JNI_OnLoad: JNI_VERSION_1_2\n");
		result = JNI_VERSION_1_2;
	}
#endif

	if(result == -1)
		return result;

	if (register_ttpod_media_MediaPlayer(env) < 0) {
		LOGE("ERROR: MediaPlayer native registration failed\n");
		return -1;
	}

	return result;
}
