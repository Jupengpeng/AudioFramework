#ifndef __ANDROIDAUDIOMANAGER__
#define __ANDROIDAUDIOMANAGER__
#include "IAudioManager.h"
#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include "YCBaseEffect.h"
#define LOG_TAG "test"
#define LOGI(f,v)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,f,v)
#define LOGI2(a)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,a)
typedef struct AudioRecoder_fields_t 
{
	jclass audio_record_class;
	jmethodID constructor;
	jmethodID getMinBufferSize;
	jmethodID startRecord;
	jmethodID stopRecord;
	jmethodID read;

} AudioRecoder_fields_t;

typedef struct S_Android_AudioRcoder
{
	jobject thiz;
	int sampRate;
	int channels;
	int sampleBit;
}S_Android_AudioRcoder;

class AndroidAudioManager:public IAudioManager
{
public:

	AndroidAudioManager();
	virtual ~AndroidAudioManager();
	virtual int Init();
	virtual int UnInit();
	virtual int SetNativeWindow(void* pNativeWindow);
	virtual int setAudioFormat(int sampleRate,int channels,int sampleBits);
	virtual int start();
	virtual int stop();


private:
	int							InitAndroidAudioSystem();
	JNIEnv*						GetJNIEnv();
	void*						m_pJVM;
	void*						m_pJEnv;
	AudioRecoder_fields_t		m_sAudioRecoderFields;
	S_Android_AudioRcoder		m_sAudioAndroidRecoder;
	S_Audio_Format				m_sAudioFormat;
	CBaseEffect*				m_pBaseEffect;	
};

#endif __ANDROIDAUDIOMANAGER__