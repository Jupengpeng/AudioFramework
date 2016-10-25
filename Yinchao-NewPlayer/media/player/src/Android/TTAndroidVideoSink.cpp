/**
* File : TTAndroidVideoSink.cpp  
* Created on : 2014-11-5
* Author : yongping.lin
* Copyright : Copyright (c) 2014 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAndroidVideoSink实现文件
*/

#include "TTOsalConfig.h"
// INCLUDES
#include <sys/system_properties.h>
#include "Android/TTAndroidVideoSink.h"
#include "TTSleep.h"
#include "TTSysTime.h"
#include "TTJniEnvUtil.h"
#include "TTDllLoader.h"
#include "TTLog.h"

#define HAL_PIXEL_FORMAT_YV12  0x32315659

extern JavaVM* gJVM;
extern jobject g_Surface;
extern jint gViewWidth;
extern jint gViewHeight;

CTTAndroidVideoSink::CTTAndroidVideoSink(CTTSrcDemux* aSrcMux, TTCBaseAudioSink* aAudioSink, TTDecoderType aDecoderType)
: TTCBaseVideoSink(aSrcMux, aAudioSink, aDecoderType)
, mNativeWnd(NULL)
, mIsYUV(0)
, mBlackEnable(1)
, mClCHandle(NULL)
, mClCAlpha(NULL)
, mClcFunc(NULL)
, mhAndroidDll(NULL)
, mbNativeWndAvailable(true)
, mpNativeWnd(NULL)
, mpANativeWindow_fromSurface(NULL)
, mpANativeWindow_release(NULL)
, mpANativeWindow_setBuffersGeometry(NULL)
, mpANativeWindow_lock(NULL)
, mpANativeWindow_unlockAndPost(NULL)
, mGraphicsDll(NULL)
, mbBitmapAvailable(false)
, mGetinfo(NULL)
, mLockpixels(NULL)
, mUnlockpixels(NULL)
, mBitmap(NULL)
{
	mCriView.Create();
	memset(&mSurfaceBuf, 0, sizeof(mSurfaceBuf));
	checkCPUFeature();
	checkYUVEnable(1);
	checkHWEnable();

	mVideoTrackClass = NULL;
	mVideoTrackObjRef = NULL;

	mJMethodConstructor = NULL;
	mJMethodInit = NULL;
	mJMethodRender = NULL;
	mJMethodSetSurface = NULL;
}

CTTAndroidVideoSink::~CTTAndroidVideoSink()
{
	closeVideoView();

	if(mClCHandle) {
		DllClose(mClCHandle);
		mClCHandle = NULL;
	}

	if (mhAndroidDll != NULL) {
		DllClose (mhAndroidDll);
		mhAndroidDll = NULL;
	}

	if (mGraphicsDll != NULL) {
		DllClose (mGraphicsDll);
		mGraphicsDll = NULL;
	}
	
	mCriView.Destroy();
}


TTInt CTTAndroidVideoSink::render()
{
	TTInt nErr = TTKErrNone;
	TTCAutoLock Lock(&mCriView);
	if(mHWDec && mVideoDecoder) {
		nErr = mVideoDecoder->setParam(TT_PID_VIDEO_RENDERBUFFER, &mSinkBuffer);
		return nErr;
	}

	if(!mbBitmapAvailable && !mbNativeWndAvailable)
		return TTKErrNotReady;

	if(mbBitmapAvailable && !mbNativeWndAvailable)
		return renderBitmap();
	
	if (!mNativeWnd) 
		return TTKErrNotReady;

	if(mIsYUV > 0){
		nErr = renderYUV();
		if(nErr) {
			mIsYUV = 0;
			checkYUVEnable(0);
			nErr = renderRGB();
		}
	} else {
		nErr = renderRGB();
	}

	return nErr;
}

TTInt CTTAndroidVideoSink::setView(void * pView)
{
	TTCAutoLock Lock(&mCriView);
	mView = pView;
	if(getPlayStatus() != EStatusStoped) {
		newVideoView();	
	}
}

TTInt CTTAndroidVideoSink::newVideoView()
{
	closeVideoView ();

	if(mIsYUV == 0 && (mClCAlpha == NULL || mClcFunc == NULL)) {
		checkYUVEnable(1);
		if(mIsYUV == 0 && (mClCAlpha == NULL || mClcFunc == NULL)) {
			return TTKErrNotReady;
		}
	}

	if(mhAndroidDll == NULL) {
		mbNativeWndAvailable = false;
		mhAndroidDll = DllLoad("libandroid.so");
		if (mhAndroidDll) {
			mpANativeWindow_fromSurface = (ANativeWindow_fromSurface_t)DllSymbol(mhAndroidDll, "ANativeWindow_fromSurface");
			mpANativeWindow_release = (ANativeWindow_release_t)DllSymbol(mhAndroidDll, "ANativeWindow_release");
			mpANativeWindow_setBuffersGeometry = (ANativeWindow_setBuffersGeometry_t) DllSymbol(mhAndroidDll, "ANativeWindow_setBuffersGeometry");
			mpANativeWindow_lock = (ANativeWindow_lock_t) DllSymbol(mhAndroidDll, "ANativeWindow_lock");
			mpANativeWindow_unlockAndPost = (ANativeWindow_unlockAndPost_t)DllSymbol(mhAndroidDll, "ANativeWindow_unlockAndPost");
			if (!mpANativeWindow_fromSurface || !mpANativeWindow_release || !mpANativeWindow_setBuffersGeometry
				|| !mpANativeWindow_lock || !mpANativeWindow_unlockAndPost)	{
				DllClose (mhAndroidDll);
				mhAndroidDll = NULL;
				mbNativeWndAvailable = false;
			} else {
				mbNativeWndAvailable = true;
			}
		} else {
			mbNativeWndAvailable = false;
		}
	}

	if(mbNativeWndAvailable == false) {
		mNativeWnd = NULL;
		return initBitmap();
	}

	TTCAutoLock Lock(&mCriView);
	LOGI("newVideoView::mView %d", (TTInt)mView);

	if(mView == NULL) {
		mNativeWnd = NULL;
		if(mVideoDecoder && mHWDec) {
			mVideoDecoder->stop();
		}
		return TTKErrNone;
	}

	CJniEnvUtil	env(gJVM);
	if (!env.getEnv()){
		mNativeWnd = NULL;
		return TTKErrNotReady;
	}
	
	mNativeWnd = (ANativeWindow *)mpANativeWindow_fromSurface(env.getEnv(), (jobject)g_Surface);
	if(mNativeWnd == NULL) {
		return TTKErrNotReady;
	}

	LOGI("newVideoView::mNativeWnd %d", (TTInt)mNativeWnd);
	if(mVideoDecoder && (mHWDec == TT_VIDEODEC_IOMX_ICS || mHWDec == TT_VIDEODEC_IOMX_JB)) {
		int nErr = mVideoDecoder->setParam(TT_PID_VIDEO_NATIVWINDOWS, mNativeWnd);
		if(nErr != TTKErrNone && nErr != TTKErrNotFound) {
			mHWDec = TT_VIDEODEC_SOFTWARE;
		} else if(nErr == TTKErrNone) {
			if(getPlayStatus() == EStatusPlaying || getPlayStatus() == EStatusPaused)
				mVideoDecoder->start();
		}
	}

	if(mVideoDecoder && mHWDec == TT_VIDEODEC_MediaCODEC_JAVA) {
		int nErr = mVideoDecoder->setParam(TT_PID_COMMON_JAVAVM, gJVM);
		if(nErr != TTKErrNone && nErr != TTKErrNotFound) {
			mHWDec = TT_VIDEODEC_SOFTWARE;
			return TTKErrNone;
		}

		void* pParam = (void *)&g_Surface;

		nErr = mVideoDecoder->setParam(TT_PID_COMMON_SURFACEOBJ, pParam);
		if(nErr != TTKErrNone && nErr != TTKErrNotFound) {
			mHWDec = TT_VIDEODEC_SOFTWARE;
		} else if(nErr == TTKErrNone) {
			if(getPlayStatus() == EStatusPlaying || getPlayStatus() == EStatusPaused)
				mVideoDecoder->start();
		}
	}

	return TTKErrNone;
}

TTInt CTTAndroidVideoSink::closeVideoView()
{
	TTCAutoLock Lock(&mCriView);
	if (mNativeWnd) {
		mpANativeWindow_release(mNativeWnd);
		mNativeWnd = 0;
	}

	if(mbBitmapAvailable) {
		videoTrack_close();
	}

	videoTrack_uninit();

	return TTKErrNone;
}

TTInt CTTAndroidVideoSink::drawBlackFrame()
{
	//int nErr = 0;
	//int format = WINDOW_FORMAT_RGBA_8888;
	//int strideMultipler = 4;
	//ANativeWindow_Buffer buffer;


	//if(mVideoFormat.Width == 0 || mVideoFormat.Height == 0 || !mNativeWnd || !mBlackEnable)	{
	//	return TTKErrNotReady;
	//}

	//nErr = mpANativeWindow_setBuffersGeometry(mNativeWnd, mVideoFormat.Width, mVideoFormat.Height, format);
	//if (nErr) return nErr;

	//memset(&buffer, 0, sizeof(ANativeWindow_Buffer));
	//nErr = mpANativeWindow_lock(mNativeWnd, &buffer, NULL);
	//if (nErr) return nErr;

	//if(buffer.bits == NULL) 
	//	return TTKErrNotFound;

	//mSurfaceBuf.Buffer[0] = (TTPBYTE)buffer.bits;
	//mSurfaceBuf.Stride[0] = buffer.stride * strideMultipler;		
	//
	//memset(buffer.bits, 0, buffer.stride * strideMultipler*buffer.height);	

	//mpANativeWindow_unlockAndPost(mNativeWnd);

	return TTKErrNone;
}

void CTTAndroidVideoSink::checkHWEnable() 
{
	TTChar szProp[64];
	memset (szProp, 0, 64);
	__system_property_get ("ro.build.version.release", szProp);
	//if (strstr (szProp, "4.0") == szProp || strstr (szProp, "4.1") == szProp || strstr (szProp, "4.2") == szProp) {
	//	mHWDec = TT_VIDEODEC_IOMX_ICS;
	//} else if(strstr (szProp, "4.3") == szProp || strstr (szProp, "4.4") == szProp || strstr (szProp, "5.") == szProp || strstr (szProp, "4.5") == szProp) {
	//	mHWDec = TT_VIDEODEC_IOMX_JB;
	//}

	if(strstr (szProp, "4.1") == szProp || strstr (szProp, "4.2") == szProp || strstr (szProp, "4.3") == szProp || strstr (szProp, "4.4") == szProp || strstr (szProp, "5.") == szProp ) {
		mHWDec = TT_VIDEODEC_MediaCODEC_JAVA;
	} 
	
	mBlackEnable = 1;
	if(strstr (szProp, "5.") == szProp || strstr (szProp, "6.") == szProp) {
		mBlackEnable = 0;
	}

	memset (szProp, 0, 64);
	__system_property_get ("ro.product.model", szProp);
	if(strstr(szProp, "HTC") == szProp && (mHWDec == TT_VIDEODEC_IOMX_ICS || mHWDec == TT_VIDEODEC_IOMX_JB))
		mHWDec = TT_VIDEODEC_SOFTWARE;
	
	if(strstr(szProp, "K-Touch") == szProp)
		mHWDec = TT_VIDEODEC_SOFTWARE;

	if(mDecoderType == EDecoderSoft)
		mHWDec = TT_VIDEODEC_SOFTWARE;

	LOGI("Andoird build release szProp = %s, mHWDec %d", szProp, mHWDec);
}

void CTTAndroidVideoSink::checkYUVEnable(int nEnable)
{
	mIsYUV = nEnable;
	
	TTChar szProp[64];
	memset (szProp, 0, 64);
	__system_property_get ("ro.build.version.release", szProp);
	if (strstr (szProp, "2.") == szProp || strstr (szProp, "1.") == szProp) {
		mIsYUV = 0;
	}

	memset (szProp, 0, 64);
	__system_property_get("ro.board.platform", szProp);
	LOGI("Andoird Platform = %s", szProp);

	if(!strcmp(szProp, "exynos4") || !strcmp(szProp, "rk30xx") || !strcmp(szProp, "hi3630")) {
		mIsYUV = 0;
	}

	if(mIsYUV == 0)
	{
		TTChar			DllFile[256];
		int len = (int)strlen(mVideoPath);
		memset(DllFile, 0, sizeof(TTChar)*256);

		strcpy(DllFile, mVideoPath); 
		if(len > 0) {
			if(DllFile[len-1] != '\\' && DllFile[len-1] != '/') strcat(DllFile, "/"); 
		}

		strcat (DllFile, "lib");
		if (mCPUType >= 7) {
			strcat (DllFile, "clconv_v7");
		} else{
			strcat (DllFile, "clconv");
		}

		strcat (DllFile, KCodecWildCard);

		if(mClCHandle) {
			DllClose(mClCHandle);
			mClCHandle = NULL;
		}
		mClCHandle = DllLoad(DllFile);
		if (mClCHandle == NULL)	{
			LOGI("could not Load Color Conversion library DllFile %s", DllFile);
			return;
		}

		mClCAlpha = (_colorConvAlpha)DllSymbol(mClCHandle, "colorConvAlpha");
		mClcFunc = (_colorConvYUV2RGB)DllSymbol(mClCHandle, "colorConvYUV2RGB");
	}
}

TTInt CTTAndroidVideoSink::renderRGB()	
{
	int nErr = 0;
	int format = WINDOW_FORMAT_RGBA_8888;
	int strideMultipler = 4;
	ANativeWindow_Buffer buffer;

	if(mVideoFormat.Width == 0 || mVideoFormat.Height == 0 || mSinkBuffer.Buffer[0] == NULL || mSinkBuffer.Stride[0] == 0)	{
		return TTKErrNotReady;
	}

	nErr = mpANativeWindow_setBuffersGeometry(mNativeWnd, mVideoFormat.Width, mVideoFormat.Height, format);
	if (nErr) return nErr;

	memset(&buffer, 0, sizeof(ANativeWindow_Buffer));
	nErr = mpANativeWindow_lock(mNativeWnd, &buffer, NULL);
	if (nErr) return nErr;

	if(buffer.bits == NULL) 
		return TTKErrNotFound;

	mSurfaceBuf.Buffer[0] = (TTPBYTE)buffer.bits;
	mSurfaceBuf.Stride[0] = buffer.stride * strideMultipler;		
	
	mClcFunc(&mSinkBuffer, &mSurfaceBuf, buffer.width, buffer.height);

	mpANativeWindow_unlockAndPost(mNativeWnd);

	return TTKErrNone;
}

TTInt CTTAndroidVideoSink::renderYUV()
{
	int nErr = 0;
	int format = HAL_PIXEL_FORMAT_YV12;
	ANativeWindow_Buffer buffer;

	if(mVideoFormat.Width == 0 || mVideoFormat.Height == 0 || mSinkBuffer.Buffer[0] == NULL || mSinkBuffer.Stride[0] == 0)	{
		return TTKErrNotReady;
	}

	nErr = mpANativeWindow_setBuffersGeometry(mNativeWnd, mVideoFormat.Width, mVideoFormat.Height, format);
	if (nErr) {
		return nErr;
	}

	memset(&buffer, 0, sizeof(ANativeWindow_Buffer));
	nErr = mpANativeWindow_lock(mNativeWnd, &buffer, NULL);
	if (nErr) {
		return nErr;
	}

	if(buffer.bits == NULL) 
		return TTKErrNotFound;

	mSurfaceBuf.Buffer[0] = (TTPBYTE)buffer.bits;
	mSurfaceBuf.Stride[0] = buffer.stride;		
	mSurfaceBuf.Buffer[2] =  mSurfaceBuf.Buffer[0] + buffer.stride * buffer.height;
	mSurfaceBuf.Stride[2] = (buffer.stride / 2 + 15) & ~15;	
	mSurfaceBuf.Buffer[1] =  mSurfaceBuf.Buffer[2] + mSurfaceBuf.Stride[2] * buffer.height / 2;
	mSurfaceBuf.Stride[1] = (buffer.stride / 2 + 15) & ~15;	

	int nX = buffer.width;
	int nY = buffer.height;
	int i = 0;

	if(mSinkBuffer.ColorType == TT_COLOR_YUV_PLANAR420 || mSinkBuffer.ColorType == TT_COLOR_YUV_YUYV422 || mSinkBuffer.ColorType == TT_COLOR_YUV_UYVY422) {
		for(i = 0; i < nY; i++) {
			memcpy(mSurfaceBuf.Buffer[0] + mSurfaceBuf.Stride[0] * i, mSinkBuffer.Buffer[0] + mSinkBuffer.Stride[0] * i, nX);
		}

		nX = nX / 2;
		nY = nY / 2;
		for(i = 0; i < nY; i++) {
			memcpy(mSurfaceBuf.Buffer[1] + mSurfaceBuf.Stride[1] * i, mSinkBuffer.Buffer[1] + mSinkBuffer.Stride[1] * i, nX);
		}

		for(i = 0; i < nY; i++) {
			memcpy(mSurfaceBuf.Buffer[2] + mSurfaceBuf.Stride[2] * i, mSinkBuffer.Buffer[2] + mSinkBuffer.Stride[2] * i, nX);
		}
	} else if(mSinkBuffer.ColorType == TT_COLOR_YUV_NV12 || mSinkBuffer.ColorType == TT_COLOR_YUV_NV21) {
		for(i = 0; i < nY; i++) {
			memcpy(mSurfaceBuf.Buffer[0] + mSurfaceBuf.Stride[0] * i, mSinkBuffer.Buffer[0] + mSinkBuffer.Stride[0] * i, nX);
		}

		nX = nX / 2;
		nY = nY / 2;
		TTPBYTE pUVByte = mSinkBuffer.Buffer[1];
		TTPBYTE pUByte = mSurfaceBuf.Buffer[2];
		TTPBYTE pVByte = mSurfaceBuf.Buffer[1];
		TTInt j = 0;
		if(mSinkBuffer.ColorType == TT_COLOR_YUV_NV12){
			for(i = 0; i < nY; i++) {
				for(j = 0; j < nX; j++) {
					*(pUByte + j) = pUVByte[2*j + 1];
					*(pVByte + j) = pUVByte[2*j];
				}
				pUVByte += mSinkBuffer.Stride[1];
				pUByte += mSurfaceBuf.Stride[2];
				pVByte += mSurfaceBuf.Stride[1];
			}				
		} else if(mSinkBuffer.ColorType == TT_COLOR_YUV_NV21) {
			for(i = 0; i < nY; i++) {
				for(j = 0; j < nX; j++) {
					*(pUByte + j) = pUVByte[2*j];
					*(pVByte + j) = pUVByte[2*j + 1];
				}
				pUVByte += mSinkBuffer.Stride[1];
				pUByte += mSurfaceBuf.Stride[2];
				pVByte += mSurfaceBuf.Stride[1];
			}	
		}
	}
	else if(mSinkBuffer.ColorType == TT_COLOR_YUV_YUYV422 || mSinkBuffer.ColorType == TT_COLOR_YUV_UYVY422) {
		TTPBYTE pSrcByte = mSinkBuffer.Buffer[0];
		TTPBYTE pSrcBytePlus = mSinkBuffer.Buffer[0] + mSinkBuffer.Stride[0];
		TTPBYTE pYByte = mSurfaceBuf.Buffer[0];
		TTPBYTE pYBytePlus = mSurfaceBuf.Buffer[0] + mSurfaceBuf.Stride[0];
		TTPBYTE pUByte = mSurfaceBuf.Buffer[2];
		TTPBYTE pVByte = mSurfaceBuf.Buffer[1];
		TTInt j = 0;
		
		nX = nX / 2;
		nY = nY / 2;

		if(mSinkBuffer.ColorType == TT_COLOR_YUV_YUYV422){
			for(i = 0; i < nY; i++) {
				for(j = 0; j < nX; j++) {
					*(pYByte + 2*j) = pSrcByte[4*j];
					*(pYByte + 2*j + 1) = pSrcByte[4*j + 2];
					*(pYBytePlus + 2*j) = pSrcBytePlus[4*j];
					*(pYBytePlus + 2*j + 1) = pSrcBytePlus[4*j + 2];
					*(pUByte + j) = (pSrcByte[4*j + 1] + pSrcBytePlus[4*j + 1]) >> 1;
					*(pVByte + j) = (pSrcByte[4*j + 3] + pSrcBytePlus[4*j + 3]) >> 1;
				}
				pSrcByte += mSinkBuffer.Stride[0]*2;
				pSrcBytePlus += mSinkBuffer.Stride[0]*2;
				pYByte += mSurfaceBuf.Stride[0]*2;
				pYBytePlus += mSurfaceBuf.Stride[0]*2;
				pUByte += mSurfaceBuf.Stride[2];
				pVByte += mSurfaceBuf.Stride[1];
			}				
		} else if(mSinkBuffer.ColorType == TT_COLOR_YUV_UYVY422) {
			for(i = 0; i < nY; i++) {
				for(j = 0; j < nX; j++) {
					*(pYByte + 2*j) = pSrcByte[4*j + 1];
					*(pYByte + 2*j+1) = pSrcByte[4*j + 3];
					*(pYBytePlus + 2*j) = pSrcBytePlus[4*j + 1];
					*(pYBytePlus + 2*j+1) = pSrcBytePlus[4*j + 3];
					*(pUByte + j) = (pSrcByte[4*j] + pSrcBytePlus[4*j]) >> 1;
					*(pVByte + j) = (pSrcByte[4*j + 2] + pSrcBytePlus[4*j + 2]) >> 1;
				}
				pSrcByte += mSinkBuffer.Stride[0]*2;
				pSrcBytePlus += mSinkBuffer.Stride[0]*2;
				pYByte += mSurfaceBuf.Stride[0]*2;
				pYBytePlus += mSurfaceBuf.Stride[0]*2;
				pUByte += mSurfaceBuf.Stride[2];
				pVByte += mSurfaceBuf.Stride[1];
			}
		}
	}

	mpANativeWindow_unlockAndPost(mNativeWnd);
	return TTKErrNone;
}

void CTTAndroidVideoSink::checkCPUFeature()
{
#if (defined CPU_ARM)
	if (android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM && android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON){
		mCPUType = 7;
		mVideoDecoder->setParam(TT_PID_VIDEO_CPU_TYPE, &mCPUType);
	} 
	else 
#endif
	{
		mCPUType = 6;
		mVideoDecoder->setParam(TT_PID_VIDEO_CPU_TYPE, &mCPUType);
	}

#if (defined CPU_ARM)
	mCPUNum = android_getCpuCount();
#else
	mCPUNum = 1;
#endif	
	LOGI("CTTAndroidVideoSink::checkCPUFeature mCPUNum %d", mCPUNum);

	mVideoDecoder->setParam(TT_PID_VIDEO_THREAD_NUM, &mCPUNum);
}

void CTTAndroidVideoSink::setJniInfo(void* aVideoTrackClass)
{
	if(aVideoTrackClass) {
		mVideoTrackClass = *(jclass*)aVideoTrackClass;
		videoTrack_init(aVideoTrackClass);
	}
}

TTInt CTTAndroidVideoSink::initBitmap()
{
	TTCAutoLock Lock(&mCriView);

	if(mVideoTrackClass == NULL)
		return TTKErrNotReady;  

	if(mGraphicsDll == NULL) {
		mbBitmapAvailable = false;
		mGraphicsDll = DllLoad("libjnigraphics.so");
		if(mGraphicsDll) {
			mGetinfo = (bitmap_getInfo)DllSymbol( mGraphicsDll , "AndroidBitmap_getInfo" );
			mLockpixels = (bitmap_lockPixels) DllSymbol ( mGraphicsDll , "AndroidBitmap_lockPixels");
			mUnlockpixels = (bitmap_unlockPixels) DllSymbol ( mGraphicsDll , "AndroidBitmap_unlockPixels");

			if (!mGetinfo || !mLockpixels || !mUnlockpixels)	{
				DllClose (mGraphicsDll);
				mGraphicsDll = NULL;
				mbBitmapAvailable = false;
				return TTKErrNotReady;
			} else {
				mbBitmapAvailable = true;
			}
		} else {
			return TTKErrNotReady;
		}	
	}

	if(mVideoTrackObjRef == NULL) {
		videoTrack_init(&mVideoTrackClass);
	}

	videoTrack_setScreenSize();
	videoTrack_setSurface();

	TTInt nErr = videoTrack_open(mVideoFormat.Width, mVideoFormat.Height);
	if(nErr != TTKErrNone){
		LOGI("videoTrack_open failt");
		return nErr;
	}

	CJniEnvUtil	env(gJVM);
	memset(&mInfo, 0, sizeof(mInfo));
	mGetinfo( env.getEnv(), mBitmap , &mInfo );

	return TTKErrNone; 
}

TTInt CTTAndroidVideoSink::renderBitmap()
{
	if(mVideoTrackClass == NULL)
		return TTKErrNotReady; 

	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();

	mLockpixels(pEnv, mBitmap, (void**)&(mSurfaceBuf.Buffer[0]));

	mSurfaceBuf.Stride[0] = mInfo.stride;	
	mClcFunc(&mSinkBuffer, &mSurfaceBuf, mInfo.width, mInfo.height);

	mUnlockpixels(pEnv, mBitmap);

	return videoTrack_render();
}

void CTTAndroidVideoSink::videoTrack_init(void* aVideoTrackClass)
{
	if(mVideoTrackClass == NULL)
		return ; 

	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	jclass videoTrackClass = *(jclass*)aVideoTrackClass;
	mJMethodConstructor = pEnv->GetMethodID(videoTrackClass, "<init>", "()V");
	if(mJMethodConstructor == NULL){
		LOGE("can't find videoTrackConstructor !");
	}

	jobject videoTrackObj = pEnv->NewObject(videoTrackClass, mJMethodConstructor);
	if(videoTrackObj == NULL){
		LOGE("can't Construct videoTrack!");
	}
	mVideoTrackObjRef = (jobject)pEnv->NewGlobalRef(videoTrackObj);	

	pEnv->DeleteLocalRef(videoTrackObj);
	LOGI("videoTrack_init Finished");
}

void CTTAndroidVideoSink::videoTrack_uninit()
{
	if(mVideoTrackClass == NULL)
		return ; 

	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	pEnv->DeleteGlobalRef(mVideoTrackObjRef);
	mVideoTrackObjRef = NULL;
}

void CTTAndroidVideoSink::videoTrack_setScreenSize()
{
	if(mVideoTrackClass == NULL || mVideoTrackObjRef == 0)
		return ; 

	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	mJMethodSetScreenSize = pEnv->GetMethodID(mVideoTrackClass, "setViewSize", "(II)V");
	if(mJMethodSetScreenSize == NULL) {
		LOGE("can't video track Screen Size!");
		return;
	}

	pEnv->CallIntMethod(mVideoTrackObjRef, mJMethodSetScreenSize, gViewWidth, gViewHeight);
}

void CTTAndroidVideoSink::videoTrack_setSurface()
{
	if(mVideoTrackClass == NULL)
		return ; 

	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	mJMethodSetSurface = pEnv->GetMethodID(mVideoTrackClass, "setSurface", "(Landroid/view/Surface;)V");
	if(mJMethodSetSurface == NULL) {
		LOGE("can't video track SetSurface!");
		return;
	}

	pEnv->CallIntMethod(mVideoTrackObjRef, mJMethodSetSurface, g_Surface);
}

int CTTAndroidVideoSink::videoTrack_open(int width, int height)
{
	if(mVideoTrackClass == NULL)
		return TTKErrNotReady; 

	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	int nErr = TTKErrNone;
	mJMethodInit = pEnv->GetMethodID(mVideoTrackClass, "init", "(II)I");
	if(mJMethodInit == NULL) {
		LOGE("can't videoTrack init function!");
		return TTKErrNotFound;
	}
	nErr = pEnv->CallIntMethod(mVideoTrackObjRef, mJMethodInit, width, height);
	if(nErr) {
		LOGE("can't videoTrack init failt!");
		return TTKErrNotReady;
	}


	jfieldID cntBitmapID = pEnv->GetFieldID(mVideoTrackClass, "mBitmap", "Landroid/graphics/Bitmap;");
	if (cntBitmapID == NULL)	{
		LOGE("can't videoTrack bitmap ID!");
		return TTKErrNotFound;
	}
	jobject		bitmapObj = pEnv->GetObjectField(mVideoTrackObjRef, cntBitmapID);

	if(mBitmap != NULL)
	{
		pEnv->DeleteGlobalRef(mBitmap);
		mBitmap = NULL;
	}	

	mBitmap = pEnv->NewGlobalRef(bitmapObj);
	return TTKErrNone;
}

void CTTAndroidVideoSink::videoTrack_close()
{
	if(mVideoTrackClass == NULL)
		return ; 

	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	if(mBitmap != NULL)
	{
		pEnv->DeleteGlobalRef(mBitmap);
		mBitmap = NULL;
	}

	mJMethodRender = NULL;
}

int CTTAndroidVideoSink::videoTrack_render()
{
	if(mVideoTrackClass == NULL)
		return TTKErrNotReady; 

	CJniEnvUtil	env(gJVM);
	JNIEnv* pEnv = env.getEnv();
	if(mJMethodRender == NULL) {
		mJMethodRender = pEnv->GetMethodID(mVideoTrackClass, "render", "()I");
		if(mJMethodRender == NULL) {
			LOGE("can't videoTrack render!");
			return TTKErrNotReady;
		}
	}

	TTInt nErr = pEnv->CallIntMethod(mVideoTrackObjRef, mJMethodRender);
	return nErr;
}