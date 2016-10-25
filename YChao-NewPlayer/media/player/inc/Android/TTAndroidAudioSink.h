/**
* File : TTAndroidAudioSink.h
* Created on : 2014-11-4
* Author : yongping.lin
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAndroidAudioSink定义文件
*/

#ifndef __TT_ANDROID_AUDIO_SINK_H__
#define __TT_ANDROID_AUDIO_SINK_H__

#include <jni.h>
#include "TTOsalConfig.h"
#include "TTBaseAudioSink.h"
#include "TTCritical.h"


typedef int (*_getOutputLatency)(uint32_t* latency, int streamType);

class CTTAndroidAudioSink : public TTCBaseAudioSink
{
public:
	CTTAndroidAudioSink(CTTSrcDemux* SrcMux, TTInt nCount);

	virtual ~CTTAndroidAudioSink();

public:
	virtual void				setJniInfo(void* aJClass);

public:
	virtual TTInt				render();
	virtual	TTInt				stop();
	virtual TTInt				freeAudioTrack();
	virtual TTInt				newAudioTrack();
	virtual TTInt				closeAudioTrack();
	virtual TTInt				setVolume(TTInt aLVolume, TTInt aRVolume);

private:
	void						audioTrack_init(void* aAudioTrackClass);
	void						audioTrack_uninit();
	int							audioTrack_open(int aSamplerate, int aChannels);
	void						audioTrack_setvolume(int aLVolume, int aRVolume);
	void						audioTrack_write(jbyte *pBuffer, int aSize);
	void						audioTrack_stop();
	void						audioTrack_close();
	int							audioTrack_updateBuffer(int aSize);
	void						audioTrack_updateEnv();
	void						audioTrack_updateCloseEnv();

	int							audioLatency();

private:
	jbyteArray					mByteRender;
	jint						mByteSize;
	JNIEnv*						mEnv;
	bool 						mNeedDetach;
	jclass						mAudioTrackClass;
	jobject						mAudioTrackObjRef;
	jmethodID					mJMethodConstructor;
	jmethodID					mJMethodStop;
	jmethodID					mJMethodOpen;
	jmethodID					mJMethodClose;
	jmethodID					mJMethodWrite;
	jmethodID					mJMethodSetVolume;
};

#endif
