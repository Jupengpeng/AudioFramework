#ifdef _LINUX_ANDROID
#include "com_yinchao_media_recoder_YCPcmPlayer.h"
#include "AYMediaPlayer.h"
#include <stdio.h>
#include <android/log.h>
#define LOG_TAG "test"
#define LOGI(f,v)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,f,v)
#define LOGI2(a)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,a)
JavaVM* gJVM = NULL;
static const char* const kClassPathName="com/yinchao/media/recoder/YCPcmPlayer";
static const char* const kClassFieldName = "mNativePcmPlayerPara";

/*
* Class:     com_yinchao_media_recoder_YCPcmPlayer
* Method:    setDataSource
* Signature: (Ljava/lang/String;[BLjava/lang/String;[B)V
*/
void JNICALL Java_com_yinchao_media_recoder_YCPcmPlayer_setDataSource
	(JNIEnv *jni_env, jobject thiz, jstring recordPath, jbyteArray recordFormat, jstring accompanyPath, jbyteArray accompanyFormat){
		IAYMediaPlayer *pPcmPlayer=NULL;
		if (createMediaPlayerInstance(NULL,&pPcmPlayer))
		{
			jni_env->GetJavaVM(&gJVM);

			//JNIEnv * env = NULL;
			//if( gJVM->AttachCurrentThread(&env, NULL) != JNI_OK )
			//{
			//	
			//}
			pPcmPlayer->setParam(PID_JAVA_VM,(void*)gJVM);
			const char*path =jni_env->GetStringUTFChars(recordPath,0);
			pPcmPlayer->setParam(PID_RECORD_FILE_PATH,(void*)(char*)path);
			path = jni_env->GetStringUTFChars(accompanyPath,0);
			pPcmPlayer->setParam(PID_BACKGROUD_FILE_PATH,(void*)(char*)path);

			AYMediaAudioFormat recordFormat;
			recordFormat.nSamplesPerSec=44100;
			recordFormat.nChannels =1;
			recordFormat.wBitsPerSample =16;
			pPcmPlayer->setParam(PID_RECORDER_AUDIO_FORMAT,(void*)&recordFormat);

			AYMediaAudioFormat backGroudFormat;
			backGroudFormat.nSamplesPerSec=44100;
			backGroudFormat.nChannels =2;
			backGroudFormat.wBitsPerSample =16;
			pPcmPlayer->setParam(PID_BACKGROUD_AUDIO_FORMAT,(void*)&backGroudFormat);

		}
		if (pPcmPlayer!=NULL)
		{
			jclass className = jni_env->FindClass(kClassPathName);
			jfieldID PcmPlayerPara = jni_env->GetFieldID(className,kClassFieldName, "I"); 
			jni_env->SetIntField(thiz,PcmPlayerPara, (int)pPcmPlayer);
			jni_env->DeleteLocalRef(className);
		}
		if (pPcmPlayer!=NULL)
		{
			pPcmPlayer->run();
		}

		LOGI("setDataSource! =%d",(int)pPcmPlayer);
}

/*
* Class:     com_yinchao_media_recoder_YCPcmPlayer
* Method:    start
* Signature: (I)V
*/
void JNICALL Java_com_yinchao_media_recoder_YCPcmPlayer_start
	(JNIEnv *jni_env, jobject thiz, jint context){
		IAYMediaPlayer* pPcmPlayer = (CAYMediaPlayer*)context;
		if (pPcmPlayer == NULL)
		{
			return;
		}
		pPcmPlayer->run();
		LOGI("start! =%d",context);
}

/*
* Class:     com_yinchao_media_recoder_YCPcmPlayer
* Method:    seek
* Signature: (II)V
*/
void JNICALL Java_com_yinchao_media_recoder_YCPcmPlayer_seek
	(JNIEnv *jni_env, jobject thiz, jint iPosition, jint context){
		IAYMediaPlayer* pPcmPlayer = (CAYMediaPlayer*)context;
		if (pPcmPlayer == NULL)
		{
			return;
		}
		pPcmPlayer->seekTo((unsigned int*)&iPosition);
		LOGI("setDataSource!=%d",context);
}

/*
* Class:     com_yinchao_media_recoder_YCPcmPlayer
* Method:    pause
* Signature: (I)V
*/
void JNICALL Java_com_yinchao_media_recoder_YCPcmPlayer_pause
	(JNIEnv *jni_env, jobject thiz, jint context){
		IAYMediaPlayer* pPcmPlayer = (CAYMediaPlayer*)context;
		if (pPcmPlayer == NULL)
		{
			return;
		}
		pPcmPlayer->pause();
		LOGI("pause!=%d",context);
}

/*
* Class:     com_yinchao_media_recoder_YCPcmPlayer
* Method:    resume
* Signature: (I)V
*/
void JNICALL Java_com_yinchao_media_recoder_YCPcmPlayer_resume
	(JNIEnv *jni_env, jobject thiz, jint context){
		IAYMediaPlayer* pPcmPlayer = (CAYMediaPlayer*)context;
		if (pPcmPlayer == NULL)
		{
			return;
		}
		pPcmPlayer->resume();
		LOGI("reusme=%d",context);
}

/*
* Class:     com_yinchao_media_recoder_YCPcmPlayer
* Method:    stop
* Signature: (I)V
*/
void JNICALL Java_com_yinchao_media_recoder_YCPcmPlayer_stop
	(JNIEnv *jni_env, jobject thiz, jint context){
		IAYMediaPlayer* pPcmPlayer = (CAYMediaPlayer*)context;
		if (pPcmPlayer == NULL)
		{
			return;
		}
		pPcmPlayer->stop();

}

/*
* Class:     com_yinchao_media_recoder_YCPcmPlayer
* Method:    setVolume
* Signature: (II)V
*/
void JNICALL Java_com_yinchao_media_recoder_YCPcmPlayer_setVolume
	(JNIEnv *jni_env, jobject thiz, jint iVolume, jint context){
		IAYMediaPlayer* pPcmPlayer = (CAYMediaPlayer*)context;
		if (pPcmPlayer == NULL)
		{
			return;
		}
		pPcmPlayer->setParam(PID_AUDIO_BACKGROUD_VOLUME,(void*)&iVolume);
}

#endif