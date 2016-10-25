/**
* File : TTAndroidVideoSink.h  
* Created on : 2014-11-5
* Author : yongping.lin
* Copyright : Copyright (c) 2014 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAndroidVideoSink定义文件
*/

#ifndef __TT_ANDROID_VIDEO_SINK_H__
#define __TT_ANDROID_VIDEO_SINK_H__

#include <stdio.h>
#include <jni.h>
#include <unistd.h>
#if (defined CPU_ARM)
#include <cpu-features.h>
#endif
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "Android/TTAndroidAudioSink.h"
#include "TTBaseVideoSink.h"
#include "TTCritical.h"
#include "ttClConv.h"

typedef void* (*ANativeWindow_fromSurface_t)(JNIEnv *env, jobject surface);
typedef void (*ANativeWindow_release_t)(void *window);
typedef int (*ANativeWindow_setBuffersGeometry_t)(void *window, int width, int height, int format);
typedef int (* ANativeWindow_lock_t)(void *window, void *outBuffer, void *inOutDirtyBounds);
typedef int (* ANativeWindow_unlockAndPost_t)(void *window);

//bitmap render define
enum AndroidBitmapFormat {
    ANDROID_BITMAP_FORMAT_NONE      = 0,
    ANDROID_BITMAP_FORMAT_RGBA_8888 = 1,
    ANDROID_BITMAP_FORMAT_RGB_565   = 4,
    ANDROID_BITMAP_FORMAT_RGBA_4444 = 7,
    ANDROID_BITMAP_FORMAT_A_8       = 8,
};

typedef struct {
    uint32_t    width;
    uint32_t    height;
    uint32_t    stride;
    int32_t     format;
    uint32_t    flags; // 0 for now
} AndroidBitmapInfo;

typedef int (* bitmap_getInfo)(JNIEnv* env, jobject jbitmap, AndroidBitmapInfo* info);
typedef int (* bitmap_lockPixels)(JNIEnv* env, jobject jbitmap, void** addrPtr);
typedef int (* bitmap_unlockPixels)(JNIEnv* env, jobject jbitmap);

// CLASSES DECLEARATION
class CTTAndroidVideoSink : public TTCBaseVideoSink
{
public:
	CTTAndroidVideoSink(CTTSrcDemux* aSrcMux, TTCBaseAudioSink* aAudioSink, TTDecoderType aDecoderType);
	virtual ~CTTAndroidVideoSink();

public:
	virtual void				setJniInfo(void* aJClass);

public:
	virtual TTInt				render();

	virtual	TTInt				setView(void * pView);

	virtual TTInt				newVideoView();

	virtual TTInt				closeVideoView();

	virtual TTInt				drawBlackFrame();

	virtual void				checkCPUFeature();
protected:
	virtual void				checkYUVEnable(int nEnable);
	virtual void				checkHWEnable();
	virtual TTInt				initBitmap();
	virtual TTInt				renderRGB();	
	virtual TTInt				renderYUV();
	virtual TTInt				renderBitmap();

private:
	void						videoTrack_init(void* aVideoTrackClass);
	void						videoTrack_uninit();
	void						videoTrack_setScreenSize();
	void						videoTrack_setSurface();
	int							videoTrack_open(int width, int height);
	void						videoTrack_close();
	int							videoTrack_render();

protected:	
	ANativeWindow*				mNativeWnd;	
	TTVideoBuffer				mSurfaceBuf;
	TTInt						mIsYUV;
	TTInt						mBlackEnable;

	RTTCritical					mCriView;

	TTHandle					mClCHandle;
	_colorConvAlpha				mClCAlpha;
	_colorConvYUV2RGB			mClcFunc;

	void*								mhAndroidDll;
	bool								mbNativeWndAvailable;
	ANativeWindow*						mpNativeWnd;
	ANativeWindow_fromSurface_t			mpANativeWindow_fromSurface;
	ANativeWindow_release_t				mpANativeWindow_release;
	ANativeWindow_setBuffersGeometry_t	mpANativeWindow_setBuffersGeometry;
	ANativeWindow_lock_t				mpANativeWindow_lock;
	ANativeWindow_unlockAndPost_t		mpANativeWindow_unlockAndPost;


	// Android bitmap render
	jclass								mVideoTrackClass;
	jobject								mVideoTrackObjRef;
	void*								mGraphicsDll;
	bool								mbBitmapAvailable;
	AndroidBitmapInfo					mInfo;
	bitmap_getInfo 						mGetinfo;
	bitmap_lockPixels 					mLockpixels;
	bitmap_unlockPixels					mUnlockpixels;
	jobject 							mBitmap;

	jmethodID							mJMethodConstructor;
	jmethodID							mJMethodInit;
	jmethodID							mJMethodRender;
	jmethodID							mJMethodSetSurface;
	jmethodID							mJMethodSetScreenSize;

};

#endif // __TT_WIN_AUDIO_SINK_H__