#include <assert.h>
#include <math.h>
#include <jni.h>
#include "STMediaPlayer.h"
#include "STLog.h"
#include "STRunTime.h"

static const STChar* kPlayBackClassPath = "com/sds/android/media/MediaPlayer";
static jmethodID   event_callback;

#define KRecordWaveBufferSampleCount (256)

class STMediaPlayerObserverImp : public ISTMediaPlayerObserver
{
public:
	STMediaPlayerObserverImp(jobject thiz, jobject weak_thiz)
	{
		jclass clazz = STRunTime::GetJNIEnv()->GetObjectClass(thiz);
		if (clazz == NULL) {
			STLOGE("Can't create STMediaPlayerObserverImp");
			STRunTime::GetJNIEnv()->ThrowNew(clazz, "Can't create STMediaPlayerObserverImp");
			return;
		}
		cls = (jclass)STRunTime::GetJNIEnv()->NewGlobalRef(clazz);
		obj  = STRunTime::GetJNIEnv()->NewGlobalRef(weak_thiz);
	}

	~STMediaPlayerObserverImp()
	{
		STRunTime::GetJNIEnv()->DeleteGlobalRef(obj);
		STRunTime::GetJNIEnv()->DeleteGlobalRef(cls);
	}

	void PlayerNotifyEvent(STNotifyMsgId aMsg, ISTMediaPlayerItf* aSender, STInt aArg)
    {
        STRunTime::GetJNIEnv()->CallStaticVoidMethod(cls, event_callback, obj, aMsg, (int)aSender, aArg, 0);
    }

private:
    jclass cls;
    jobject obj;
};

static void jni_init(JNIEnv* env, jobject thiz)
{
	jclass cls = env->FindClass(kPlayBackClassPath);
	if (cls == NULL) {
		STLOGE("Can't find %s", kPlayBackClassPath);
	}

	event_callback = env->GetStaticMethodID(cls, "postEventFromNative", "(Ljava/lang/Object;IIILjava/lang/Object;)V");
}

static int jni_createPlayer(JNIEnv* env, jobject thiz, jobject weak_this)
{
    return (int)(new STMediaPlayer(*new STMediaPlayerObserverImp(thiz, weak_this)));
}

static void jni_releasePlayer(JNIEnv* env, jobject thiz, int playerRef)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	if(player != NULL)
	{
		ISTMediaPlayerObserver* observer = player->GetAttachMediaPlayerObserver();
		player->Stop();
		SAFE_RELEASE(player);
		SAFE_DELETE(observer);
	}
}

static void jni_start(JNIEnv* env, jobject thiz, int playerRef)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	player->Play();
}

static int jni_stop(JNIEnv* env, jobject thiz, int playerRef)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	return player->Stop();
}

static jint jni_setDataSource(JNIEnv* env, jobject thiz, int playerRef, jstring url, jstring params)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);

	const STChar* tmpUrl = env->GetStringUTFChars(url, NULL);
	const STChar* tmpParams = params == NULL ? NULL : env->GetStringUTFChars(params, NULL);

	STInt ret = player->SetDataSource(tmpUrl, tmpParams);

	if (tmpParams != NULL)
	{
		env->ReleaseStringUTFChars(params, tmpParams);
	}
	env->ReleaseStringUTFChars(url, tmpUrl);
	STLOGE("setDataSource:%d", ret);
	//STLOGE("setDataSource_chenbin:%d", ret);
	return ret;
}

static jint jni_setDataSourceAsync(JNIEnv* env, jobject thiz, int playerRef, jstring url, jstring params)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);

	const STChar* tmpUrl = env->GetStringUTFChars(url, NULL);
	const STChar* tmpParams = params == NULL ? NULL : env->GetStringUTFChars(params, NULL);

	STInt ret = player->SetDataSourceAsync(tmpUrl, tmpParams);

	if (tmpParams != NULL)
	{
		env->ReleaseStringUTFChars(params, tmpParams);
	}
	env->ReleaseStringUTFChars(url, tmpUrl);

	STLOGE("setDataSourceAsync:%d", ret);
	//STLOGE("setDataSourceAsync_chenbin:%d", ret);

	return ret;
}

static void jni_pause(JNIEnv* env, jobject thiz, int playerRef)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	player->Pause();
}

static void jni_resume(JNIEnv* env, jobject thiz, int playerRef)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	player->Resume();
}

static void jni_seek(JNIEnv* env, jobject thiz, int playerRef, int aPos)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	player->Seek(aPos);
}

static void jni_setAccompanimentVolume(JNIEnv* env, jobject thiz, int playerRef, int aVolume)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	player->SetAccompanimentVolume(aVolume);
}

static void jni_setRecorderVolume(JNIEnv* env, jobject thiz, int playerRef, int aVolume)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	player->SetRecorderVolume(aVolume);
}

static int jni_duration(JNIEnv* env, jobject thiz, int playerRef)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	return player->Duration();
}

static int jni_position(JNIEnv* env, jobject thiz, int playerRef)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	return player->GetPosition();
}

static int jni_playStatus(JNIEnv* env, jobject thiz, int playerRef)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	return player->GetPlayStatus();
}

static int jni_wave(JNIEnv* env, jobject thiz, int playerRef, jshortArray arr, int samples)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	STInt nChannels = 0;
	STInt16 pData[2*samples];
	STInt nErr = player->GetCurFreqAndWave(NULL, pData, samples, nChannels);
	if (nErr != 0)
	{
		memset(pData, 0, sizeof(pData));
	}
	env->SetShortArrayRegion(arr, 0, 2*samples, pData);
	return nErr;
}


//add by bin.chen脉冲文件路径设置
static int jni_setAudioImpulsePath(JNIEnv* env, jobject thiz,int playerRef, jstring path, jint flag)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);

	const STChar* tmpPath = env->GetStringUTFChars(path, NULL);
	int iFlag = (int)flag;
	STInt nErr = player->setAudioImpulsePath(tmpPath,iFlag);
	env->ReleaseStringUTFChars(path, tmpPath);
	STLOGE("setAudioImpulsePath:%s,flag=%d", tmpPath,iFlag);
	return nErr;
}


static int jni_getDownloadPercent(JNIEnv* env, jobject thiz,int playerRef)
{
	STInt pos = -1;
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	pos = player->GetDownloadPercent();
	//STLOGE("jni_GetDownloadPercent:download pos= %d",pos);
	return pos;
}

static int jni_rhythm(JNIEnv* env, jobject thiz, int playerRef, jintArray arr)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);
	STInt nChannels = 0;
	STInt16 tWave[2*KRecordWaveBufferSampleCount];
	memset(tWave, 0, sizeof(tWave));
	STInt nErr = player->GetCurRecordWave(NULL, tWave, KRecordWaveBufferSampleCount, nChannels);
	STInt nPos = player->GetPosition();
	env->SetIntArrayRegion(arr, 0, 1, &nPos);

	double total = 0;
	if (nChannels == 1)
	{
		for(STInt i = 0; i < KRecordWaveBufferSampleCount; i++)
		{
			double tmp = tWave[i] >= 0 ? (tWave[i] + 1) : (-tWave[i]);
			total += 10 * log(tmp/32767) / log(10);
		}
	}
	else
	{
		for(STInt i = 0; i < KRecordWaveBufferSampleCount * 2; i += 2)
		{
			double tmp = tWave[i] >= 0 ? (tWave[i] + 1) : (-tWave[i]);
			total += 10 * log(tmp/32767) / log(10);
		}
	}

	STInt nLevel = (total / KRecordWaveBufferSampleCount + 0.5f);
	env->SetIntArrayRegion(arr, 1, 1, &nLevel);

//	STInt nErr = player->GetCurFreqAndWave(tOrigin, tWave, KRecordWaveBufferSampleCount, nChannels);
//	if (nErr == STKErrNone)
//	{
//		player->GetCurRecordWave(tRecord, tWave, KRecordWaveBufferSampleCount, nChannels);
//		STInt nPos = player->GetPosition();
//		env->SetIntArrayRegion(arr, 0, 1, &nPos);
//
//		STInt nCount = KRecordWaveBufferSampleCount / 8;
//
//		STDouble nEXY = 0;
//		for (STInt i = 0; i < nCount; i++)
//		{
//			nEXY += tOrigin[i] * tRecord[i];
//		}
//
//		STDouble nEX = 0;
//		for (STInt i = 0; i < nCount; i++)
//		{
//			nEX += tOrigin[i];
//		}
//
//		STDouble nEX2 = 0;
//		for (STInt i = 0; i < nCount; i++)
//		{
//			nEX2 += tOrigin[i] * tOrigin[i];
//		}
//
//		STDouble nEY = 0;
//		for (STInt i = 0; i < nCount; i++)
//		{
//			nEY += tRecord[i];
//		}
//
//		STDouble nEY2 = 0;
//		for (STInt i = 0; i < nCount; i++)
//		{
//			nEY2 += tRecord[i] * tRecord[i];
//		}
//
//		STDouble tmp1 = nEXY - nEX * nEY / nCount;
//		STDouble tmp21 = nEX2 - nEX * nEX / nCount;
//		STDouble tmp22 = nEY2 - nEY * nEY / nCount;
//
//		STInt nLevel = 0;
//		if (tmp22 != 0 && tmp21 != 0)
//		{
//			STDouble cor = tmp1 / sqrt(tmp21 * tmp22);
//			nLevel = (STInt)(cor * 100);
//			STLOGE("cor:%f", cor);
//		}
//		env->SetIntArrayRegion(arr, 1, 1, &nLevel);
//	}

	return nErr;
}

static int jni_switch2Stream(JNIEnv* env, jobject thiz, int playerRef, jstring streamName)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);

	const STChar* tmpName = env->GetStringUTFChars(streamName, NULL);
	STInt ret = player->Switch2Stream(tmpName);
	env->ReleaseStringUTFChars(streamName, tmpName);

	return ret;
}

static jstring jni_getCurStreamName(JNIEnv* env, jobject thiz, int playerRef)
{
	STMediaPlayer* player = (STMediaPlayer*)playerRef;
	STASSERT(player != NULL);

	const STChar* pStreamName = player->GetCurStreamName();
	return (pStreamName != NULL) ? env->NewStringUTF(pStreamName) : NULL;
}

static JNINativeMethod playback_interface_Methods[] =
{
	{"init", 	    		"()V",            												(void *)jni_init},
	{"createPlayer", 	    "(Ljava/lang/Object;)I",            							(void *)jni_createPlayer},
	{"releasePlayer", 	    "(I)V",            												(void *)jni_releasePlayer},
	{"start", 	    		"(I)V",            												(void *)jni_start},
	{"stop", 	    		"(I)I",            												(void *)jni_stop},
	{"pause", 	    		"(I)V",            												(void *)jni_pause},
	{"resume", 	    		"(I)V",            												(void *)jni_resume},
	{"setDataSource", 	    "(ILjava/lang/String;Ljava/lang/String;)I",            			(void *)jni_setDataSource},
	{"setDataSourceAsync", 	"(ILjava/lang/String;Ljava/lang/String;)I",            			(void *)jni_setDataSourceAsync},
	{"seek", 				"(II)V",            											(void *)jni_seek},
	{"setAccompanimentVolume", 	"(II)V",            										(void *)jni_setAccompanimentVolume},
	{"setRecorderVolume", 	"(II)V",            											(void *)jni_setRecorderVolume},
	{"switch2Stream", 		"(ILjava/lang/String;)I",            							(void *)jni_switch2Stream},
	{"getCurStreamName", 	"(I)Ljava/lang/String;",            							(void *)jni_getCurStreamName},
	{"duration", 			"(I)I",            												(void *)jni_duration},
	{"position", 			"(I)I",            												(void *)jni_position},
	{"playStatus", 			"(I)I",            												(void *)jni_playStatus},
	{"wave", 				"(II[S)I",            											(void *)jni_wave},
	{"rhythm", 				"(I[I)I",            											(void *)jni_rhythm},
	{"setAudioImpulsePath", "(ILjava/lang/String;I)I",            							(void *)jni_setAudioImpulsePath},
	{"getDownloadPercent", "(I)I",            											    (void *)jni_getDownloadPercent},
};

static int register_interface(JNIEnv *env)
{
	jclass playback_cls = env->FindClass(kPlayBackClassPath);
	if (playback_cls == NULL) {
		STLOGE("Can't find %s", kPlayBackClassPath);
		return -1;
	}

	if (env->RegisterNatives(playback_cls, playback_interface_Methods, sizeof(playback_interface_Methods)/sizeof(playback_interface_Methods[0])) != JNI_OK) {
		STLOGE("register_interface fail");
		return -1;
	}
	return 0;
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = -1;
	jclass entityclass = NULL;
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		STLOGE("STMediaPlayer JNI_OnLoad GetEnv failed");
		goto fail;
	}

	STASSERT(env != NULL);

	if (register_interface(env) < 0) {
		STLOGE("STMediaPlayer JNI_OnLoad register_interface failed");
		goto fail;
	}

	result = JNI_VERSION_1_4;

	STRunTime::Init(vm);

fail:
	return result;
}
