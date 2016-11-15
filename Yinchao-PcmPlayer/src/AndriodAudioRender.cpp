#include "AndriodAudioRender.h"
#ifdef _LINUX_ANDROID
#include <jni.h>
#include <stdio.h>
#include <string.h>
AndroidAudioRender::AndroidAudioRender()
{
    m_pJVM = NULL;
    m_pJEnv = NULL;
    m_iChannelCount = 0;
    m_iSampleRate = 0;
    m_iSampleBits = 0;
	m_iCurVolumeInPer = 0;
}

AndroidAudioRender::~AndroidAudioRender()
{
    UnInit();
}

int AndroidAudioRender::Init()
{
    InitAndroidAudioSystem();
    GetAndroidTrackFromAudioSpec();
    return 0;
}

int AndroidAudioRender::UnInit()
{
    JavaVM * pJava_vm = NULL;
	//ULULOGI("before AndroidAudioRender Stop");
    Stop();

    //ULULOGI("after AndroidAudioRender Stop");
    AndroidAudioTrackFree();
	//ULULOGI("after AndroidAudioTrackFree");
    if(m_pJVM != NULL)
    {
        pJava_vm = (JavaVM *)m_pJVM;
        pJava_vm->DetachCurrentThread();    
    }
	
    return 0;
}
int AndroidAudioRender::SetAudioFormat(AYMediaAudioFormat* pAudioFormat)
{
    m_iSampleBits = pAudioFormat->wBitsPerSample;
    m_iChannelCount = pAudioFormat->nChannels;
    m_iSampleRate = pAudioFormat->nSamplesPerSec;

    m_sAndroidAudioSpec.stream_type = S_Android_AudioTrack_Spec::STREAM_MUSIC;
    m_sAndroidAudioSpec.sample_rate_in_hz = m_iSampleRate;
    if(pAudioFormat->nChannels == 1)
    {
        m_sAndroidAudioSpec.channel_config = S_Android_AudioTrack_Spec::CHANNEL_OUT_MONO;
    }

	if(pAudioFormat->nChannels == 2)
	{
	    m_sAndroidAudioSpec.channel_config = S_Android_AudioTrack_Spec::CHANNEL_OUT_STEREO;
	}
	
	//ULULOGI("the SampleRate:%u, the channel:%u, the SampleBits:%u", pAudioFormat->nSamplesPerSec, pAudioFormat->nChannels, pAudioFormat->wBitsPerSample);
    switch(m_iSampleBits)
    {
        case 8:
        {
            m_sAndroidAudioSpec.audio_format = S_Android_AudioTrack_Spec::ENCODING_PCM_8BIT;
            break;
        }
        case 16:
        {
            m_sAndroidAudioSpec.audio_format = S_Android_AudioTrack_Spec::ENCODING_PCM_16BIT;
            break;
        }
    }
    m_sAndroidAudioSpec.stream_type = S_Android_AudioTrack_Spec::STREAM_MUSIC;
    m_sAndroidAudioSpec.mode = S_Android_AudioTrack_Spec::MODE_STREAM;
    Init();
    return 0;
}

int AndroidAudioRender::SetNativeWindow(void*  pJVM)
{
	if(pJVM == NULL)
	{
	    return 1;
	}
    m_pJVM = pJVM;
    return 0;
}

int AndroidAudioRender::Start()
{	
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return -1;
    }

    //ULULOGI("before pEnv->CallVoidMethod");
    pEnv->CallVoidMethod(m_sAndroidAudioTrack.thiz, m_sAndroidAudioFields.play);
    if (pEnv->ExceptionCheck()) 
    {
        //ULULOGI("Android_AudioTrack_play: play: Exception:");
        pEnv->ExceptionDescribe();
        pEnv->ExceptionClear();
        return -1;
    }
    return 0;
}

int AndroidAudioRender::Pause()
{
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return -1;
    }

    //ULULOGI("Android_AudioTrack_pause");
    pEnv->CallVoidMethod(m_sAndroidAudioTrack.thiz, m_sAndroidAudioFields.pause);
    if (pEnv->ExceptionCheck()) 
    {
        //ULULOGI("Android_AudioTrack_pause: pause: Exception:");
        pEnv->ExceptionDescribe();
        pEnv->ExceptionClear();
        return -1;
    }
    return 0;
}

int AndroidAudioRender::Stop()
{
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return -1;
    }

    pEnv->CallVoidMethod(m_sAndroidAudioTrack.thiz, m_sAndroidAudioFields.stop);
    if (pEnv->ExceptionCheck()) 
    {
        //ULULOGI("Android_AudioTrack_stop: stop: Exception:");
        pEnv->ExceptionDescribe();
        pEnv->ExceptionClear();
        return -1;
    }
    return 0;
}

int AndroidAudioRender::Render(unsigned char*   pData, unsigned int  ulDataSize)
{	
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return -1;
    }

    if (ulDataSize <= 0)
    {
        return ulDataSize;		
    }

    int reserved = GetReserveBuffer(ulDataSize);
    if (reserved < ulDataSize) 
    {
        //ULULOGI("SDL_Android_AudioTrack_reserve_buffer failed %d < %d", reserved, ulDataSize);
        return AUDIO_COMMIT_NEED_RETRY;
    }

    pEnv->SetByteArrayRegion(m_sAndroidAudioTrack.buffer, 0, ulDataSize, (jbyte*) pData);
    if (pEnv->ExceptionCheck()) 
    {
        //ULULOGI("Android_AudioTrack_write_byte: SetByteArrayRegion: Exception:");
        if (pEnv->ExceptionCheck()) 
        {
            pEnv->ExceptionDescribe();
            pEnv->ExceptionClear();
        }
        return -1;
    }

    int retval = pEnv->CallIntMethod(m_sAndroidAudioTrack.thiz, m_sAndroidAudioFields.write_byte,
    m_sAndroidAudioTrack.buffer, 0, ulDataSize);
    if (pEnv->ExceptionCheck()) 
    {
        //ULULOGI("SDL_Android_AudioTrack_write_byte: write_byte: Exception:");
        if (pEnv->ExceptionCheck()) 
        {
            pEnv->ExceptionDescribe();
            pEnv->ExceptionClear();
        }
        return -1;
    }



    return 0;
}

int AndroidAudioRender::Flush()
{
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return -1;
    }

    //ULULOGI("SDL_Android_AudioTrack_flush");
    pEnv->CallVoidMethod(m_sAndroidAudioTrack.thiz, m_sAndroidAudioFields.flush);
    //ULULOGI("Android_AudioTrack_flush()=void");
    if (pEnv->ExceptionCheck()) 
    {
        //ULULOGI("SDL_Android_AudioTrack_flush: flush: Exception:");
        pEnv->ExceptionDescribe();
        pEnv->ExceptionClear();
        return -1;
    }
    return 0;
}

int AndroidAudioRender::GetPlayingTime(uint64*  pullTime)
{
    return 0;
}

int AndroidAudioRender::GetBufferTime(uint64*  pullTime)
{
    return 0;
}

int AndroidAudioRender::SetCallBack(void*   pCallbackFunc)
{
    return 0;
}

int AndroidAudioRender::GetRemainSize()
{
    return 0;
}

int AndroidAudioRender::InitAndroidAudioSystem()
{
    jclass clazz;
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return -1;
    }
    
    //ULULOGI("InitAndroidAudioSystem Init");
    
    clazz = pEnv->FindClass("android/media/AudioTrack");
    //ULU_CHECK_RET(clazz, -1, "missing AudioTrack");

    //ULULOGI("FindClass android/media/AudioTrack ok!");

    // FindClass returns LocalReference
    m_sAndroidAudioFields.clazz = (jclass)pEnv->NewGlobalRef(clazz);
    //ULU_CHECK_RET(m_sAndroidAudioFields.clazz, -1, "AudioTrack NewGlobalRef failed");
    pEnv->DeleteLocalRef(clazz);

    //ULULOGI("NewGlobalRef AudioTrack ok!");

    m_sAndroidAudioFields.constructor = pEnv->GetMethodID(m_sAndroidAudioFields.clazz, "<init>", "(IIIIII)V");
    //ULU_CHECK_RET(m_sAndroidAudioFields.constructor, -1, "missing AudioTrack.<init>");
    //ULULOGI("Get m_sAndroidAudioFields.constructor  AudioTrack ok!");

    m_sAndroidAudioFields.getMinBufferSize = pEnv->GetStaticMethodID(m_sAndroidAudioFields.clazz, "getMinBufferSize", "(III)I");
    //ULU_CHECK_RET(m_sAndroidAudioFields.getMinBufferSize, -1, "missing AudioTrack.getMinBufferSize");

    m_sAndroidAudioFields.getMaxVolume = pEnv->GetStaticMethodID(m_sAndroidAudioFields.clazz, "getMaxVolume", "()F");
    //ULU_CHECK_RET(m_sAndroidAudioFields.getMaxVolume, -1, "missing AudioTrack.getMaxVolume");

    m_sAndroidAudioFields.getMinVolume = pEnv->GetStaticMethodID(m_sAndroidAudioFields.clazz, "getMinVolume", "()F");
    //ULU_CHECK_RET(m_sAndroidAudioFields.getMinVolume, -1, "missing AudioTrack.getMinVolume");

    m_sAndroidAudioFields.getNativeOutputSampleRate = pEnv->GetStaticMethodID(m_sAndroidAudioFields.clazz, "getNativeOutputSampleRate", "(I)I");
    //ULU_CHECK_RET(m_sAndroidAudioFields.getNativeOutputSampleRate, -1, "missing AudioTrack.getNativeOutputSampleRate");

    m_sAndroidAudioFields.play = pEnv->GetMethodID(m_sAndroidAudioFields.clazz, "play", "()V");
    //ULU_CHECK_RET(m_sAndroidAudioFields.play, -1, "missing AudioTrack.play");

    m_sAndroidAudioFields.pause = pEnv->GetMethodID(m_sAndroidAudioFields.clazz, "pause", "()V");
    //ULU_CHECK_RET(m_sAndroidAudioFields.pause, -1, "missing AudioTrack.pause");

    m_sAndroidAudioFields.flush = pEnv->GetMethodID(m_sAndroidAudioFields.clazz, "flush", "()V");
    //ULU_CHECK_RET(m_sAndroidAudioFields.flush, -1, "missing AudioTrack.flush");

    m_sAndroidAudioFields.stop = pEnv->GetMethodID(m_sAndroidAudioFields.clazz, "stop", "()V");
    //ULU_CHECK_RET(m_sAndroidAudioFields.stop, -1, "missing AudioTrack.stop");

    m_sAndroidAudioFields.release = pEnv->GetMethodID(m_sAndroidAudioFields.clazz, "release", "()V");
    //ULU_CHECK_RET(m_sAndroidAudioFields.release, -1, "missing AudioTrack.release");

    m_sAndroidAudioFields.write_byte = pEnv->GetMethodID(m_sAndroidAudioFields.clazz, "write", "([BII)I");
    //ULU_CHECK_RET(m_sAndroidAudioFields.write_byte, -1, "missing AudioTrack.write");

    m_sAndroidAudioFields.setStereoVolume = pEnv->GetMethodID(m_sAndroidAudioFields.clazz, "setStereoVolume", "(FF)I");
    //ULU_CHECK_RET(m_sAndroidAudioFields.setStereoVolume, -1, "missing AudioTrack.setStereoVolume");

    //ULULOGI("android.media.AudioTrack class loaded");
    return 0;
}

int AndroidAudioRender::AudiotrackGetMinBufferSize()
{
    S_Android_AudioTrack_Spec*   pspec = &(m_sAndroidAudioSpec);
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return -1;
    }

    int retval = pEnv->CallStaticIntMethod(m_sAndroidAudioFields.clazz, m_sAndroidAudioFields.getMinBufferSize,
        (int) pspec->sample_rate_in_hz,
        (int) pspec->channel_config,
        (int) pspec->audio_format);
    if (pEnv->ExceptionCheck()) 
    {
        //ULULOGI("audiotrack_get_min_buffer_size: getMinBufferSize: Exception:");
        pEnv->ExceptionDescribe();
        pEnv->ExceptionClear();
        return -1;
    }

    //ULULOGI("audiotrack_get_min_buffer_size: %u", retval);
    return retval;
}

int AndroidAudioRender::GetAndroidTrackFromAudioSpec()
{
    bool   bRet = false;
    float   fRetVal = 0;
    int min_buffer_size = 0;
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return -1;
    }

    switch (m_sAndroidAudioSpec.channel_config) 
    {
        case S_Android_AudioTrack_Spec::CHANNEL_OUT_MONO:
        {
            //ULULOGI("Android_AudioTrack: %s", "CHANNEL_OUT_MONO");
            break;			
        }
        case S_Android_AudioTrack_Spec::CHANNEL_OUT_STEREO:
        {
            //ULULOGI("Android_AudioTrack: %s", "CHANNEL_OUT_STEREO");
            break;
        }

        default:
        {
            //ULULOGI("Android_AudioTrack_new_from_spec: invalid channel %d", m_sAndroidAudioSpec.channel_config);
            return -1;			
        }
    }

    switch (m_sAndroidAudioSpec.audio_format) 
    {
        case S_Android_AudioTrack_Spec::ENCODING_PCM_16BIT:
        {
            //ULULOGI("Android_AudioTrack: %s", "ENCODING_PCM_16BIT");
            break;	
        }
        case S_Android_AudioTrack_Spec::ENCODING_PCM_8BIT:
        {
            //ULULOGI("Android_AudioTrack: %s", "ENCODING_PCM_8BIT");
            break;
        }
        default:
        {
            //ULULOGI("SDL_Android_AudioTrack_new_from_spec: invalid format %d", m_sAndroidAudioSpec.audio_format);
            return -1;
        }
    }

    memset(&m_sAndroidAudioTrack, 0, sizeof(S_Android_AudioTrack));
    m_sAndroidAudioTrack.spec = m_sAndroidAudioSpec;
    min_buffer_size = AudiotrackGetMinBufferSize();
    jobject thiz = pEnv->NewObject(m_sAndroidAudioFields.clazz, m_sAndroidAudioFields.constructor,
    (int) m_sAndroidAudioTrack.spec.stream_type,
    (int) m_sAndroidAudioTrack.spec.sample_rate_in_hz,
    (int) m_sAndroidAudioTrack.spec.channel_config,
    (int) m_sAndroidAudioTrack.spec.audio_format,
    (int) min_buffer_size,
    (int) m_sAndroidAudioTrack.spec.mode);
    if (!thiz || pEnv->ExceptionCheck()) 
    {
        //ULULOGI("Android_AudioTrack_new: NewObject: Exception:");
        if (pEnv->ExceptionCheck()) 
        {
            pEnv->ExceptionDescribe();
            pEnv->ExceptionClear();
        }
        return -1;
    }

    m_sAndroidAudioTrack.min_buffer_size = min_buffer_size;
    m_sAndroidAudioTrack.spec.buffer_size_in_bytes = min_buffer_size;
    if(AudiotrackGetMaxVolume(fRetVal) == true)
    {
        m_sAndroidAudioTrack.max_volume = fRetVal;		
    }

    if(AudiotrackGetMinVolume(fRetVal) == true)
    {
        m_sAndroidAudioTrack.min_volume = fRetVal;		
    }

	//ULULOGI("the min volume:%f, the max volume:%f", m_sAndroidAudioTrack.min_volume, m_sAndroidAudioTrack.max_volume);
    m_sAndroidAudioTrack.thiz = pEnv->NewGlobalRef(thiz);
    pEnv->DeleteLocalRef(thiz);
    return 0;
}

bool  AndroidAudioRender::AudiotrackGetMaxVolume(float&  fMaxVolume)
{
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return false;
    }

    //ULULOGI("audiotrack_get_max_volume");
    float retval = pEnv->CallStaticFloatMethod(m_sAndroidAudioFields.clazz, m_sAndroidAudioFields.getMaxVolume);
    if (pEnv->ExceptionCheck()) 
    {
        //ULULOGI("audiotrack_get_max_volume: getMaxVolume: Exception:");
        pEnv->ExceptionDescribe();
        pEnv->ExceptionClear();
        return false;
    }

    fMaxVolume = retval;
    return true;
}

bool  AndroidAudioRender::AudiotrackGetMinVolume(float&  fMinVolume)
{
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return false;
    }
    
    //ULULOGI("audiotrack_get_min_volume");
    float retval = pEnv->CallStaticFloatMethod(m_sAndroidAudioFields.clazz, m_sAndroidAudioFields.getMinVolume);
    if (pEnv->ExceptionCheck()) 
    {
        //ULULOGI("audiotrack_get_min_volume: getMinVolume: Exception:");
        pEnv->ExceptionDescribe();
        pEnv->ExceptionClear();
        return false;
    }

    fMinVolume = retval;
    return true;
}

int  AndroidAudioRender::GetReserveBuffer(unsigned int ulDataSize)
{
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return -1;
    }

    if (m_sAndroidAudioTrack.buffer && ulDataSize <= m_sAndroidAudioTrack.buffer_capacity)
    {
        return ulDataSize;		
    }

    if (m_sAndroidAudioTrack.buffer) 
    {
        pEnv->DeleteGlobalRef(m_sAndroidAudioTrack.buffer);
        m_sAndroidAudioTrack.buffer = NULL;
        m_sAndroidAudioTrack.buffer_capacity = 0;
    }

    int capacity = ULUMAX(ulDataSize, m_sAndroidAudioTrack.min_buffer_size);
    jbyteArray buffer = pEnv->NewByteArray(capacity);
    if (!buffer || pEnv->ExceptionCheck()) 
    {
        //ULULOGI("Android_AudioTrack_reserve_buffer: NewByteArray: Exception:");
        if (pEnv->ExceptionCheck()) 
        {
            pEnv->ExceptionDescribe();
            pEnv->ExceptionClear();
        }
        return -1;
    }

    m_sAndroidAudioTrack.buffer_capacity = capacity;
    m_sAndroidAudioTrack.buffer = (jbyteArray)pEnv->NewGlobalRef(buffer);
    pEnv->DeleteLocalRef(buffer);
    return capacity;
}

int AndroidAudioRender::SetVolume(int iVolume)
{
	int  retval = 0;
	int  iVolumeInPer = 0;
	float    fVolumeInput = 0.0;

	JNIEnv * pEnv = GetJNIEnv();
	if(pEnv == NULL)
	{
		return -1;
	}

	if(iVolume <= 0)
	{
		iVolumeInPer = 0;
	}
	else
	{
		if(iVolume > 100)
		{
			iVolumeInPer = 100;
		}
		else
		{
			iVolumeInPer = iVolume;
		}
	}

	fVolumeInput = (((float)iVolumeInPer)/100.0)*(m_sAndroidAudioTrack.max_volume-m_sAndroidAudioTrack.min_volume)+ m_sAndroidAudioTrack.min_volume;

	//ULULOGI("Set the Volume value:%f", fVolumeInput);

	retval = pEnv->CallIntMethod(m_sAndroidAudioTrack.thiz, m_sAndroidAudioFields.setStereoVolume, fVolumeInput, fVolumeInput);
	if (pEnv->ExceptionCheck()) 
	{
		//ULULOGI("audiotrack_set_stereo_volume: write_byte: Exception:");
		if (pEnv->ExceptionCheck()) 
		{
			pEnv->ExceptionDescribe();
			pEnv->ExceptionClear();
		}
		return -1;
	}

	//ULULOGI("Set the Volume value done");
	m_iCurVolumeInPer = iVolumeInPer;
	return 0;
}

int AndroidAudioRender::GetVolume()
{
	return m_iCurVolumeInPer;
}

int AndroidAudioRender::GetLatency(){

	JNIEnv * pEnv = GetJNIEnv();

    m_sAndroidAudioFields.getLatency= pEnv->GetMethodID(m_sAndroidAudioFields.clazz, "getLatency", "()I");
    //ULU_CHECK_RET(m_sAndroidAudioFields.getLatency, -1, "missing AudioTrack.getLatency");

	
	int latency = pEnv->CallIntMethod(m_sAndroidAudioTrack.thiz, m_sAndroidAudioFields.getLatency);
	if (pEnv->ExceptionCheck()) 
	{
		//ULULOGI("SDL_Android_AudioTrack_getLatency: getLatency: Exception:");
		if (pEnv->ExceptionCheck()) 
		{
			pEnv->ExceptionDescribe();
			pEnv->ExceptionClear();
		}
		return -1;
	}
	return latency;
}
int  AndroidAudioRender::AndroidAudioTrackRelease()
{
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return -1;
    }

    //ULULOGI("Android_AudioTrack_release");
    pEnv->CallVoidMethod(m_sAndroidAudioTrack.thiz, m_sAndroidAudioFields.release);
    if (pEnv->ExceptionCheck()) 
    {
        //ULULOGI("Android_AudioTrack_release: release: Exception:");
        pEnv->ExceptionDescribe();
        pEnv->ExceptionClear();
        return -1;
    }

    return 0;	
}

void  AndroidAudioRender::AndroidAudioTrackFree()
{
    JNIEnv * pEnv = GetJNIEnv();
    if(pEnv == NULL)
    {
        return;
    }

    if (m_sAndroidAudioTrack.buffer) 
    {
        pEnv->DeleteGlobalRef(m_sAndroidAudioTrack.buffer);
        m_sAndroidAudioTrack.buffer = NULL;
    }

    m_sAndroidAudioTrack.buffer_capacity = 0;
    if (m_sAndroidAudioTrack.thiz) 
    {
        AndroidAudioTrackRelease();
        pEnv->DeleteGlobalRef(m_sAndroidAudioTrack.thiz);
        m_sAndroidAudioTrack.thiz = NULL;
    }

    return;	
}

JNIEnv*  AndroidAudioRender::GetJNIEnv()
{
    JavaVM * pJava_vm = NULL;
    JNIEnv * pEnv = NULL;

    if(m_pJVM == NULL)
    {
        //ULULOGI("JVM is NULL");
        return NULL;
    }

    pJava_vm = (JavaVM *)m_pJVM;
    if(m_pJEnv == NULL)
    {
        pJava_vm->AttachCurrentThread(&pEnv,NULL);
        m_pJEnv = (void*)pEnv;
        //ULULOGI("Get m_pJEnv value");
    }

    pEnv = (JNIEnv *)m_pJEnv;	
    return pEnv;
}
#endif
