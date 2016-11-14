#include "AndroidAudioManager.h"


int AndroidAudioManager::SetNativeWindow(void* pNativeWindow)
{
	m_pJVM =pNativeWindow;
	return 0;
}

int AndroidAudioManager::Init()
{
	return InitAndroidAudioSystem();
}

int AndroidAudioManager::UnInit()
{

}

int AndroidAudioManager::setAudioFormat(int sampleRate,int channels,int sampleBits)
{
	m_sAudioFormat.sampleRate = sampleRate;
	m_sAudioFormat.channels = channels;
	m_sAudioFormat.sampleBits =sampleBits;
	if (m_pBaseEffect==NULL)
	{
		m_pBaseEffect =new CBaseEffect(sampleRate,channels,sampleBits);

	}
	return 1;
}

int AndroidAudioManager::start()
{
	JNIEnv * jni_env = GetJNIEnv();
	if (jni_env==NULL)
	{
		return -1;
	}
	LOGI("start recording! jni_env=%d",jni_env);
	//start recording
	jni_env->CallVoidMethod(m_sAudioAndroidRecoder.thiz, m_sAudioRecoderFields.startRecord);
	return 1;
}

int AndroidAudioManager::stop()
{

	JNIEnv * jni_env = GetJNIEnv();
	if (jni_env==NULL)
	{
		return -1;
	}
	//stop recording
	LOGI("stop recording! jni_env=%d",jni_env);
	jni_env->CallVoidMethod(m_sAudioAndroidRecoder.thiz, m_sAudioRecoderFields.stopRecord);

	return 1;
}

JNIEnv*  AndroidAudioManager::GetJNIEnv()
{
	JavaVM * pJava_vm = NULL;
	JNIEnv * pEnv = NULL;

	if(m_pJVM == NULL)
	{
		return NULL;
	}

	pJava_vm = (JavaVM *)m_pJVM;
	if(m_pJEnv == NULL)
	{
		pJava_vm->AttachCurrentThread(&pEnv,NULL);
		m_pJEnv = (void*)pEnv;
	}

	pEnv = (JNIEnv *)m_pJEnv;	
	return pEnv;
}

int AndroidAudioManager::InitAndroidAudioSystem()
{
	jclass clazz;
	jobject thiz;
	JNIEnv * jni_env = GetJNIEnv();
	if (jni_env==NULL)
	{
		return -1;
	}

	clazz = jni_env->FindClass("android/media/AudioRecord");
	m_sAudioRecoderFields.audio_record_class =(jclass)jni_env->NewGlobalRef(clazz);

	m_sAudioRecoderFields.constructor = jni_env->GetMethodID(m_sAudioRecoderFields.audio_record_class, "<init>",
		"(IIIII)V");

	m_sAudioRecoderFields.getMinBufferSize = jni_env->GetStaticMethodID(m_sAudioRecoderFields.audio_record_class,
		"getMinBufferSize", "(III)I");

	jint buff_size
		= jni_env->CallStaticIntMethod(m_sAudioRecoderFields.audio_record_class,
		m_sAudioRecoderFields.getMinBufferSize,
		m_sAudioFormat.sampleRate,
		16,
		2);

	thiz= jni_env->NewObject(m_sAudioRecoderFields.audio_record_class, m_sAudioRecoderFields.constructor,
		7,
		m_sAudioFormat.sampleRate,
		16,
		2,
		buff_size);
	m_sAudioAndroidRecoder.thiz =jni_env->NewGlobalRef(thiz);
	m_sAudioRecoderFields.startRecord = jni_env->GetMethodID(m_sAudioRecoderFields.audio_record_class, "startRecording",
		"()V");

	m_sAudioRecoderFields.stopRecord = jni_env->GetMethodID(m_sAudioRecoderFields.audio_record_class, "stop",
		"()V");

	m_sAudioRecoderFields.read =jni_env->GetMethodID(m_sAudioRecoderFields.audio_record_class, "read", "([BII)I");



	LOGI("init recoder done jni=%d",jni_env);
	return 1;
}

AndroidAudioManager::AndroidAudioManager()
{
	m_pJVM =0;
	m_pJEnv =0;
	m_pBaseEffect =NULL;
}

AndroidAudioManager::~AndroidAudioManager()
{

}

/*int AndroidAudioManager::readAndroidSample(int16_t *dataIn,int size)
{
	jbyteArray	read_buff;
	jbyte*		audio_bytes;
	int			nread=0;
	JNIEnv * jni_env = GetJNIEnv();
	if (jni_env==NULL)
	{
		return -1;
	}
	read_buff = jni_env->NewByteArray(size*4);;
	nread = jni_env->CallIntMethod(m_sAudioAndroidRecoder.thiz,m_sAudioRecoderFields.read, read_buff, 0, size*4);
	jni_env->GetByteArrayRegion(read_buff, 0, nread,audio_bytes);
	dataIn = (int16_t*)audio_bytes;
	return nread;
}

int AndroidAudioManager::writeAndroidSample(int16_t *dataOut,int size)
{
	return 1;

}*/



