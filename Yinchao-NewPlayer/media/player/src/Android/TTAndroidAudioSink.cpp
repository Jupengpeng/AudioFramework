/**
* File : TTAndroidAudioSink.cpp
* Created on : 2014-11-4
* Author : yongping.lin
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAndroidAudioSink实锟斤拷
*/

#include <dlfcn.h>
#include "Android/TTAndroidAudioSink.h"
#include "TTJniEnvUtil.h"
#include "TTSysTime.h"
#include "TTLog.h"

#define MAX_VOLUME 65535
#define KMAX_ZEROBUFFER_COUNT 4

extern JavaVM* gJVM;

CTTAndroidAudioSink::CTTAndroidAudioSink(CTTSrcDemux* SrcMux, TTInt nCount)
: TTCBaseAudioSink(SrcMux, nCount)
{
	mLVolume = MAX_VOLUME / 2;
	mRVolume = MAX_VOLUME / 2;

	mByteRender = NULL;
	mByteSize = 0;
	mEnv = NULL;
	mNeedDetach = false;
	mJMethodConstructor = NULL;
	mJMethodOpen = NULL;
	mJMethodStop = NULL;
	mJMethodClose = NULL;
	mJMethodWrite = NULL;
	mJMethodSetVolume = NULL;
}

CTTAndroidAudioSink::~CTTAndroidAudioSink()
{
	stop();
	audioTrack_uninit();
}

TTInt CTTAndroidAudioSink::newAudioTrack()
{
	LOGE("TTInt CTTAndroidAudioSink::newAudioTrack()");
//	closeAudioTrack();
	LOGE("TTInt CTTAndroidAudioSink::newAudioTrack()");
	return audioTrack_open(mAudioFormat.SampleRate, mAudioFormat.Channels);
}

TTInt CTTAndroidAudioSink::closeAudioTrack()
{
	LOGE("TTInt CTTAndroidAudioSink::closeAudioTrack()");
	audioTrack_close();
}

TTInt CTTAndroidAudioSink::render()
{
	//TTInt64 nStart = GetTimeOfDay();
	TTInt nErr = TTKErrNone;
	if(mAudioProcess == NULL)
		return TTKErrNotFound;

	mCritTime.Lock();
	TTBool bSeeking = mSeeking;
	mCritTime.UnLock();
	
	if(mProcessCount > 1) {
		mSinkBuffer.pBuffer = NULL; 
		mSinkBuffer.nSize = 0; 
	}else {
		mSinkBuffer.pBuffer = mSinkBuf;
		mSinkBuffer.nSize = mSinkBufLen;
	}

	mSinkBuffer.llTime = mCurPos;
	mSinkBuffer.nFlag = 0;
	//TTInt64 nEnd = GetTimeOfDay();
	//TTInt64 nBeforeGet = nEnd - nEnd;

	//nStart = GetTimeOfDay();
	nErr = mAudioProcess->getOutputBuffer(&mSinkBuffer);
	//nEnd = GetTimeOfDay();
	//TTInt64 nGetDataTime = nEnd - nEnd;
	//TTInt64 nAfterRender = 0;
	if (nErr == TTKErrNone){
		if(mSinkBuffer.nFlag & TT_FLAG_BUFFER_EOS) {
			setEOS();
			nErr = TTKErrEof;
		} else if(mSinkBuffer.nFlag & TT_FLAG_BUFFER_NEW_FORMAT){
			audioFormatChanged();
			startOne(-1);
		} else {
			if(mSinkBuffer.nFlag & TT_FLAG_BUFFER_TIMESTAMP_RESET)	{
				if(mObserver) {
					mObserver->pObserver(mObserver->pUserData, ENotifyTimeReset, (TTInt)mSinkBuffer.llTime, 0, NULL);
				}
			}

			if(mRenderNum == 0) {
				audioFormatChanged();
			}

			if(ETTAudioFadeNone != getFadeStatus())
				fadeOutInHandle();

			audioTrack_write((jbyte *)mSinkBuffer.pBuffer, mSinkBuffer.nSize);

			//nStart = GetTimeOfDay();
			mCritTime.Lock();
			if(mSeeking && !bSeeking) {
				mCritTime.UnLock();
				return TTKErrNone;
			}
			mCurPos = mSinkBuffer.llTime + mFrameDuration;
			mRenderPCM += mSinkBuffer.nSize;

			if(mRenderNum == 0)	{
				mAudioSystemTime = 0;
				if(mSeeking) {
					if(mObserver) {
						mObserver->pObserver(mObserver->pUserData, ENotifySeekComplete, TTKErrNone, 0, NULL);
					}
					mSeeking = false;
				}
				mFrameDuration = mSinkBuffer.nSize*1000/(mAudioFormat.SampleRate*mAudioFormat.Channels*sizeof(short));
			}

			mRenderNum++;
			mAudioBufStartTime = mSinkBuffer.llTime;
			mAudioSysStartTime = GetTimeOfDay();
			if(mAudioSystemTime == 0) {
				mAudioSystemTime = GetTimeOfDay();
				mAudioBufferTime = mAudioBufStartTime;
			}
			mCritTime.UnLock();

			startOne(-1);

			//nEnd = GetTimeOfDay();
			//nAfterRender = nEnd - nStart;
		}
	} else if(nErr == TTKErrEof) {
		setEOS();
	}
	else if(nErr == TTKErrOverflow) {
		SAFE_FREE(mSinkBuf);
		mSinkBufLen = mSinkBuffer.nSize*3/2;
		mSinkBuf = (TTPBYTE)malloc(mSinkBufLen);

		startOne(-1);
	} else if(nErr == TTKErrUnderflow) {
		startOne(2);
	} else if(nErr == TTKErrFormatChanged) {
		audioFormatChanged();
		startOne(0);
	} else{
		startOne(2);
	}

	//LOGI("CTTAndroidAudioSink::render nErr %d, nBeforeGet %lld, nGetDataTime %lld,  AfterRender %lld", nErr, nBeforeGet, nGetDataTime, nAfterRender);

	return nErr;
}

TTInt CTTAndroidAudioSink::setVolume(TTInt aLVolume, TTInt aRVolume)
{
	audioTrack_setvolume(aLVolume, aRVolume);
	return TTKErrNone;	
}

TTInt CTTAndroidAudioSink::stop()
{
	TTCBaseAudioSink::stop();
	closeAudioTrack();
	return TTKErrNone;
}

TTInt CTTAndroidAudioSink::freeAudioTrack()
{
	TTCBaseAudioSink::freeAudioTrack();
	if(mByteRender) {
		CJniEnvUtil	env(gJVM);
		JNIEnv* pEnv = env.getEnv();
		pEnv->DeleteGlobalRef(mByteRender);
		mByteRender = NULL;
		mByteSize = 0;
	}
	audioTrack_updateCloseEnv();
	return TTKErrNone;
}

void CTTAndroidAudioSink::setJniInfo(void* aAudioTrackClass)
{
	mAudioTrackClass = *(jclass*)aAudioTrackClass;
	audioTrack_init(aAudioTrackClass);
}

void CTTAndroidAudioSink::audioTrack_init(void* aAudioTrackClass)
{
	LOGI("audioTrack_init");
	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	jclass audioTrackClass = *(jclass*)aAudioTrackClass;
	mJMethodConstructor = pEnv->GetMethodID(audioTrackClass, "<init>", "()V");
	if(mJMethodConstructor == NULL){
		LOGE("can't find audioTrackConstructor !");
	}

	jobject audioTrackObj = pEnv->NewObject(audioTrackClass, mJMethodConstructor);
	if(audioTrackObj == NULL){
		LOGE("can't Construct audioTrack!");
	}
	mAudioTrackObjRef = (jobject)pEnv->NewGlobalRef(audioTrackObj);

	pEnv->DeleteLocalRef(audioTrackObj);
	LOGI("audioTrack_init Finished");
}

void CTTAndroidAudioSink::audioTrack_uninit()
{
	LOGI("audioTrack_uninit");
	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
    jmethodID JMethodDestroy = pEnv->GetMethodID(mAudioTrackClass, "audioDestroy", "()V");
	if(JMethodDestroy != NULL) {
		pEnv->CallVoidMethod(mAudioTrackObjRef, JMethodDestroy);
	}

	if(mAudioTrackObjRef) {
		pEnv->DeleteGlobalRef(mAudioTrackObjRef);
		mAudioTrackObjRef = NULL;
	}
	LOGI("audioTrack_uninit OK");
}

int CTTAndroidAudioSink::audioTrack_open(int aSamplerate, int aChannels)
{

	LOGE("CTTAndroidAudioSink::audioTrack_open(int aSamplerate, int aChannels)");
	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	int nErr = TTKErrNone;
	mJMethodOpen = pEnv->GetMethodID(mAudioTrackClass, "audioOpen", "(II)I");
	if(mJMethodOpen == NULL) {
		LOGE("can't audioTrack open!");
		return TTKErrNotFound;
	}
	nErr = pEnv->CallIntMethod(mAudioTrackObjRef, mJMethodOpen, aSamplerate, aChannels);

	if(mProcessCount <= 1) {
		mSinkBufLen = aSamplerate*aChannels*sizeof(short);
		mSinkBuf = (TTPBYTE)malloc(mSinkBufLen);
	}

	jfieldID cntMinBufferID = pEnv->GetFieldID(mAudioTrackClass, "mMinBufferSize", "I");
	if (cntMinBufferID == NULL)	{
		LOGE("can't minbuffersize!");
		return TTKErrNotFound;
	}
	jint nMinBufferSize = (jint)pEnv->GetIntField(mAudioTrackObjRef, cntMinBufferID);

	int nLatency = audioLatency();
	if(nLatency == 0) {
		mAudioOffSetTime = nMinBufferSize*1000*3/(aSamplerate*aChannels*2*2);
	} else {
		mAudioOffSetTime = nMinBufferSize*1000/(aSamplerate*aChannels*2) + nLatency;
	}

	return nErr;
}

void CTTAndroidAudioSink::audioTrack_setvolume(int aLVolume, int aRVolume)
{
	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	mJMethodSetVolume = pEnv->GetMethodID(mAudioTrackClass, "audioSetVolume", "(II)V");
	if(mJMethodSetVolume == NULL) {
		LOGE("can't setVolume!");
		return;
	}
	pEnv->CallVoidMethod(mAudioTrackObjRef, mJMethodSetVolume, aLVolume, aRVolume);
}

void CTTAndroidAudioSink::audioTrack_close()
{
	LOGE("void CTTAndroidAudioSink::audioTrack_close()");
	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	mJMethodClose = pEnv->GetMethodID(mAudioTrackClass, "audioClose", "()V");
	if(mJMethodClose == NULL) {
		LOGE("can't audioTrack close!");
		return;
	}

	pEnv->CallVoidMethod(mAudioTrackObjRef, mJMethodClose);
	mJMethodWrite = NULL;
	SAFE_FREE(mSinkBuf);
}

void CTTAndroidAudioSink::audioTrack_stop()
{
	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	mJMethodStop = pEnv->GetMethodID(mAudioTrackClass, "audioStop", "()V");
	if(mJMethodStop == NULL) {
		LOGE("can't audioTrack stop!");
		return;
	}

	pEnv->CallVoidMethod(mAudioTrackObjRef, mJMethodStop);
	mJMethodWrite = NULL;
}

void CTTAndroidAudioSink::audioTrack_write(jbyte *pBuffer, int aSize)
{
	//TTInt64 nStart = GetTimeOfDay();
	if(mEnv == NULL) {
		audioTrack_updateEnv();
		if(mEnv == NULL)
			return;
	}

	if(mJMethodWrite == NULL) {
		mJMethodWrite = mEnv->GetMethodID(mAudioTrackClass, "writeData", "([BI)V");
		if(mJMethodWrite == NULL) {
			LOGE("can't audioTrack write!");
			return;
		}
	}
	//TTInt64 nEnd = GetTimeOfDay();
	//TTInt64 nGetEnvTime = nEnd - nStart;

	//nStart = GetTimeOfDay();
	TTInt nSize = audioTrack_updateBuffer(aSize);
	if(nSize < aSize) {
		LOGE("can't audioTrack write buffer small!");
		return;
	}

	mEnv->SetByteArrayRegion(mByteRender, 0, aSize, pBuffer);
	//nEnd = GetTimeOfDay();
	//TTInt64 nGetDataTime = nEnd - nStart;

	//nStart = GetTimeOfDay();
	mEnv->CallVoidMethod(mAudioTrackObjRef, mJMethodWrite, mByteRender, aSize);
	//nEnd = GetTimeOfDay();
	//TTInt64 nRenderTime = nEnd - nStart;
	//LOGI("CTTAndroidAudioSink::audioTrack_write  nGetEnvTime %lld, nGetDataTime %lld, nRenderTime %lld", nGetEnvTime, nGetDataTime, nRenderTime);
}

int CTTAndroidAudioSink::audioLatency()
{
	void* hMediaDll = dlopen("libmedia.so", RTLD_NOW);	
	if(hMediaDll == NULL)
		return 0;
	uint32_t afLatency = 0;
	int stream_type = -1;
	int nErr = 0;
	_getOutputLatency pgetOutputLatency = (_getOutputLatency)dlsym(hMediaDll, "_ZN7android11AudioSystem16getOutputLatencyEPj19audio_stream_type_t");
	if(pgetOutputLatency != NULL)
		nErr = pgetOutputLatency(&afLatency, stream_type);
	dlclose(hMediaDll);

	if(nErr == 0)
		return afLatency;
	return 0;
}

int CTTAndroidAudioSink::audioTrack_updateBuffer(int aSize) 
{
	if(aSize <= mByteSize)
		return aSize;
		
	if(aSize > mByteSize) {
		if(mByteRender) {
			mEnv->DeleteGlobalRef(mByteRender);
			mByteRender = NULL;
			mByteSize = 0;
		}
	}

	int capacity = aSize;
    jbyteArray buffer = mEnv->NewByteArray(capacity);
    if (!buffer || mEnv->ExceptionCheck()) {
        if (mEnv->ExceptionCheck()) {
            mEnv->ExceptionDescribe();
            mEnv->ExceptionClear();
        }
        return -1;
    }

	mByteSize = aSize;
    mByteRender = (jbyteArray)mEnv->NewGlobalRef(buffer);
    mEnv->DeleteLocalRef(buffer);
    return capacity;
}

void CTTAndroidAudioSink::audioTrack_updateEnv()
{
	mNeedDetach = false;
	switch (gJVM->GetEnv((void**)&mEnv, JNI_VERSION_1_6)) { 
		case JNI_OK: 
			break; 
		case JNI_EDETACHED: 
			mNeedDetach = true;
			if (gJVM->AttachCurrentThread(&mEnv, NULL) != 0) { 
				LOGE("callback_handler: failed to attach current thread");
				break;
			}
			break; 
		case JNI_EVERSION: 
			LOGE("Invalid java version"); 
			break;
	}

	LOGI("mEnv Initialize, mEnv %d",(long)mEnv);
}

void CTTAndroidAudioSink::audioTrack_updateCloseEnv()
{
	LOGI("mEnv UnInitialize, mEnv %d", (long)mEnv);
	
	if(mEnv) {
		if (mNeedDetach) 
			gJVM->DetachCurrentThread();
		mEnv = NULL;
		mNeedDetach = false;
	}
}
