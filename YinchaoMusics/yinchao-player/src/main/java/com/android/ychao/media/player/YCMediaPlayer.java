package com.android.ychao.media.player;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewGroup.LayoutParams;


public class YCMediaPlayer implements IMediaPlayer {

    private static final float PERCENT = 0.01f;
    private final static String TAG = "YCMediaPlayer";

    static {
        try {
            System.loadLibrary("osal");
            System.loadLibrary("mediaplayer");
        } catch (UnsatisfiedLinkError error) {
            Log.wtf(TAG,"System.loadLibrary:failed.",error);
        }
    }

    private static final int MIN_HARDWARE_VOLUME = -990;
    private EventHandler mEventHandler;
    private boolean mPlayerReleased = false;
    private long mNativePlayerPara = 0;

    /**
     * 频谱显示相关.
     */
    private Surface mSurface;
    private SurfaceHolder mSurfaceHolder;
    private SurfaceView mSurfaceView;

    private int mWidth = 0;
    private int mHeight = 0;

    private int mViewWidth = 0;
    private int mViewHeight = 0;

    /**
     * constructors.
     *
     * @param headerBytes 不注
     * @param plugInPath  动库文件路径.
     */
    public YCMediaPlayer(byte[] headerBytes, String plugInPath) {
        mEventHandler = new EventHandler(this, Looper.myLooper());

        int maxSamplerate = YCAudioTrack.maxOutputSamplerate();
        nativeSetup(this, headerBytes, maxSamplerate, plugInPath);
    }

	/*
     * A native method that is implemented by the 'hello-jni' native library,
	 * which is packaged with this application.
	 */

    private native void nativeSetup(Object weakThis, byte[] headerBytes, int samplerate, String pluginPath);

    private native void nativeRelease(long context);

    private native int nativeSetDataSourceSync(long context, String url, int flag);

    private native int nativeSetDataSourceAsync(long context, String url, int flag);

    private native int nativeSetSurface(long context);

    private native int nativeGetCurFreqAndWave(long context, short[] freqArr, short[] waveArr, int sampleNum);

    private native int nativeGetCurFreq(long context, short[] freqArr, int freqNum);

    private native int nativeGetCurWave(long context, short[] waveArr, int waveNum);

    private native void nativeSetVolume(long context, int lVolume, int rVolume);

    private native int nativeStop(long context);

    private native void nativeSetCacheFilePath(long context, String cacheFilePath);

    private native void nativeSetAudioEffectLowDelay(long context, boolean enable);

    private native void nativeCongfigProxyServer(long context, String ip, int port, String authenkey, boolean useProxy);

    private native void nativeSetActiveNetWorkType(long context, int type);

    private native void nativeSetDecoderType(long context, int type);

    private native int nativePlay(long context);

    private native void nativePause(long context, boolean enable);

    private native void nativeResume(long context, boolean enable);

    private native int nativeSetPosition(long context, int aPos, int flag);

    private native void nativeSetPlayRange(long context, int aStart, int aEnd);

    private native int nativeGetPosition(long context);

    private native int nativeDuration(long context);

    private native int nativeSize(long context);

    private native int nativeBufferedSize(long context);

    private native int nativeBufferBandWidth(long context);

    private native int nativeBufferedPercent(long context);

    private native int nativeBufferBandPercent(long context);

    private native int nativeGetStatus(long context);

    private native void nativeClearScrren(long context, int aClear);

    /**
     * 设置网络类型
     *
     * @param type 类型
     */
    public void setActiveNetWorkType(int type) {
        nativeSetActiveNetWorkType(mNativePlayerPara, type);
    }

    public static final int EDecoderDefault = 0x00;
    public static final int EDecoderSoft = 0x01;

    /**
     * 设置解码类型
     *
     * @param type 类型
     */
    public void setDecoderType(int type) {
        nativeSetDecoderType(mNativePlayerPara, type);
    }

    /**
     * 设置缓存文件路径
     *
     * @param cacheFilePath 文件路径
     */
    @Override
    public void setCacheFilePath(String cacheFilePath) {
        nativeSetCacheFilePath(mNativePlayerPara, cacheFilePath);
    }

    /**
     * play stream
     *
     * @return operation status
     */
    public int play() {
        return nativePlay(mNativePlayerPara);
    }

    /**
     * pause stream
     */
    public void pause() {
        nativePause(mNativePlayerPara, false);
    }

    /**
     * resume stream
     */
    public void resume() {
        nativeResume(mNativePlayerPara, false);
    }

    /**
     * seek Operation.
     *
     * @param aPos seek position in milliseconds
     */
    public int setPosition(int aPos, int flag) {
        return nativeSetPosition(mNativePlayerPara, aPos, flag);
    }

    /**
     * cue cut setPlayRange Operation.
     *
     * @param aStart 范围
     * @param aEnd   范围结束
     */
    public void setPlayRange(int aStart, int aEnd) {
        nativeSetPlayRange(mNativePlayerPara, aStart, aEnd);
    }

    /**
     * get current position.
     *
     * @return current position in milliseconds
     */
    public int getPosition() {
        return nativeGetPosition(mNativePlayerPara);
    }

    /**
     * get stream duration.
     *
     * @return stream duration in milliseconds
     */
    public int duration() {
        return nativeDuration(mNativePlayerPara);
    }

    /**
     * get stream size.
     *
     * @return stream size in bytes
     */
    public int size() {
        return nativeSize(mNativePlayerPara);
    }

    /**
     * get buffered size.
     *
     * @return buffered size in bytes
     */
    public int bufferedSize() {
        return nativeBufferedSize(mNativePlayerPara);
    }

    /**
     * get buffered Percent.
     *
     * @return stream duration in %
     */
    public int bufferedPercent() {
        return nativeBufferedPercent(mNativePlayerPara);
    }

    /**
     * get stream status.
     *
     * @return stream status
     */
    public int getStatus() {
        return nativeGetStatus(mNativePlayerPara);
    }


    @Override
    public float getBufferPercent() {
        return nativeBufferedPercent(mNativePlayerPara) * PERCENT;
    }

    @Override
    public int getBufferSize() {
        return nativeBufferedSize(mNativePlayerPara);
    }

    public int bufferedBandWidth() {
        return nativeBufferBandWidth(mNativePlayerPara);
    }


    /**
     * 获取文件大小
     *
     * @return 文件大小
     */
    @Override
    public int getFileSize() {
        return nativeSize(mNativePlayerPara);
    }


    /**
     * get current wave and spectrum data
     *
     * @param aFreqarr   spectrum data	output
     * @param aWavearr   wave data		output
     * @param aSampleNum samples num
     * @return operation status
     */
    @Override
    public int getCurFreqAndWave(short[] aFreqarr, short[] aWavearr, int aSampleNum) {
        int nRet = -1;
        synchronized (this) {
            if (!mPlayerReleased) {
                nRet = nativeGetCurFreqAndWave(mNativePlayerPara, aFreqarr, aWavearr, aSampleNum);
            }
        }

        return nRet;
    }

    /**
     * get current spectrum data
     *
     * @param aFreqarr spectrum data	output
     * @param aFreqNum number of frequency bands   input
     * @return operation status
     */
    @Override
    public int getCurFreq(short[] aFreqarr, int aFreqNum) {
        int nRet = -1;
        synchronized (this) {
            if (!mPlayerReleased) {
                nRet = nativeGetCurFreq(mNativePlayerPara, aFreqarr, aFreqNum);
            }
        }

        return nRet;
    }

    /**
     * get current wave data
     *
     * @param aWavearr wave data		output
     * @param aWaveNum waveform points   input
     * @return operation status
     */
    @Override
    public int getCurWave(short[] aWavearr, int aWaveNum) {
        int nRet = -1;
        synchronized (this) {
            if (!mPlayerReleased) {
                nRet = nativeGetCurWave(mNativePlayerPara, aWavearr, aWaveNum);
            }
        }

        return nRet;
    }

    /**
     * set audio volume.
     *
     * @param aLVolume left channel volume.
     * @param aRVolume right channel volume.
     */
    @Override
    public void setVolume(float aLVolume, float aRVolume) {
        int leftVolume = (int) ((1.0f - aLVolume) * MIN_HARDWARE_VOLUME);
        int rightVolume = (int) ((1.0f - aRVolume) * MIN_HARDWARE_VOLUME);
        nativeSetVolume(mNativePlayerPara, leftVolume, rightVolume);
    }

    /**
     * stop stream
     */
    @Override
    public void stop() {
        if (0 != nativeStop(mNativePlayerPara)) {
            throw new IllegalStateException("can not be stopped!");
        }
    }

    private int setDataSource(String aUrl, int flag, boolean aSync) {
        int nErr;
        Log.e("YCMediaPlayer", "setDataSource:context=" + mNativePlayerPara);
        if (aSync) {
            nErr = nativeSetDataSourceSync(mNativePlayerPara, aUrl, flag);
        } else {
            nErr = nativeSetDataSourceAsync(mNativePlayerPara, aUrl, flag);
        }
        return nErr;
    }

    /**
     * set stream source synchronized
     *
     * @param aUrl stream source path.
     * @return operation status
     */
    public int setDataSource(String aUrl, int flag) {
        return setDataSource(aUrl, flag, true);
    }

    /**
     * set stream source asynchronized
     *
     * @param aUrl stream source path.
     */
    @Override
    public void setDataSourceAsync(String aUrl, int flag) {
        if (0 != setDataSource(aUrl, flag, false)) {
            throw new IllegalStateException("can not setDataSourceAsync !");
        }
    }

    /**
     * release player resource
     */
    @Override
    public void release() {
        synchronized (this) {
            nativeRelease(mNativePlayerPara);
            mPlayerReleased = true;
        }
    }

    @Override
    public void setChannelBalance(float balance) {
        setVolume(1.0f - balance, 1.0f + balance);
    }

    public static final int OPEN_DEFAULT = 0x00;
    public static final int OPEN_BUFFER = 0x01;
    public static final int OPEN_PRE_LOADING = 0x10;
    public static final int OPEN_PRE_LOADED = 0x20;

    public static final int SEEK_FAST = 0x00;
    public static final int SEEK_CORRECT = 0x01;

    /**
     * wrong operation
     */
    public static final int MEDIA_NOP = 0;

    /**
     * prepare operation
     */
    public static final int MEDIA_PREPARE = 1;

    /**
     * play operation
     */
    public static final int MEDIA_PLAY = 2;

    /**
     * play complete operation
     */
    public static final int MEDIA_COMPLETE = 3;

    /**
     * pause operation
     */
    public static final int MEDIA_PAUSE = 4;

    /**
     * close operation
     */
    public static final int MEDIA_CLOSE = 5;

    /**
     * play exception operation
     */
    public static final int MEDIA_EXCEPTION = 6;

    /**
     * update stream duration operation
     */
    public static final int MEDIA_UPDATE_DURATION = 7;

    public static final int MEDIA_SEEK_COMPLETED = 11;

    public static final int MEDIA_AUDIOFORMAT_CHANGED = 12;

    public static final int MEDIA_VIDEOFORMAT_CHANGED = 13;

    /**
     * notify stream is Buffering
     */
    public static final int MEDIA_BUFFERING_START = 16;

    /**
     * notify stream Buffer completed
     */
    public static final int MEDIA_BUFFERING_DONE = 17;

    /**
     * DNS解析完成
     */
    public static final int MEDIA_DNS_DONE = 18;

    /**
     * 连接完成
     */
    public static final int MEDIA_CONNECT_DONE = 19;

    /**
     * 接收Http头成
     */
    public static final int MEDIA_HTTP_HEADER_RECEIVED = 20;

    /**
     * notify stream start receive data
     */
    public static final int MEDIA_START_RECEIVE_DATA = 21;

    /**
     * notify stream prefetch completed
     */
    public static final int MEDIA_PREFETCH_COMPLETED = 22;

    /**
     * notify stream cache completed
     */
    public static final int MEDIA_CACHE_COMPLETED = 23;

    /**
     * notify stream fade out start
     */
    public static final int MEDIA_FADEOUT_START = 24;

    /**
     * stream status starting
     */
    public static final int MEDIASTATUS_STARTING = 1;

    /**
     * stream status playing
     */
    public static final int MEDIASTATUS_PLAYING = 2;

    /**
     * stream status paused
     */
    public static final int MEDIASTATUS_PAUSED = 3;

    /**
     * stream status stopped
     */
    public static final int MEDIASTATUS_STOPPED = 4;

    /**
     * stream status prepared
     */
    public static final int MEDIASTATUS_PREPARED = 5;

    /**
     * max sample num to be fetch
     */
    public static final int MAX_SAMPLE_NUM = 1024;

    /**
     * min sample num to be fetch
     */
    public static final int MIN_SAMPLE_NUM = 256;

    private class EventHandler extends Handler {
        public EventHandler(YCMediaPlayer mp, Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message aMsg) {
            if (mMediaPlayerNotifyEventListener != null) {
                mMediaPlayerNotifyEventListener.onMediaPlayerNotify(aMsg.what, aMsg.arg1, aMsg.arg2, aMsg.obj);
            }
        }
    }

    private static void postEventFromNative(Object aMediaplayerRef, int aMsg, int aArg1, int aArg2, Object aObj) {
        YCMediaPlayer mp = (YCMediaPlayer) aMediaplayerRef;
        if (mp == null) {
            return;
        }

        if (mp.mEventHandler != null) {
            Message m = mp.mEventHandler.obtainMessage(aMsg, aArg1, aArg2, aObj);
            mp.mEventHandler.sendMessage(m);
        }
    }

    /**
     * 波形显示.
     *
     * @param sv 当前SurfaceView
     */
    @Override
    public void setView(SurfaceView sv) {

    }

    /**
     * 波形显示.
     *
     * @param width
     * @param Height
     */
    @Override
    public void setViewSize(int width, int Height) {

    }

    /**
     * 波形显示.
     *
     * @param width
     * @param Height
     */
    @Override
    public void videoSizeChanged(int width, int Height) {

    }

    /**
     * 波形显示.
     *
     * @param aClear
     */
    @Override
    public void ClearScreen(int aClear) {

    }


    /**
     * MediaPlayerListener
     *
     * @author zhan.liu
     * @version 2.4.0
     * @since 2011-07-25
     */
    public interface OnMediaPlayerNotifyEventListener {

        /**
         * @param aMsgId 消息ID
         * @param aArg1  整型参数1
         * @param aArg2  整型参数2
         * @param aObj   对象参数
         */
        void onMediaPlayerNotify(int aMsgId, int aArg1, int aArg2, Object aObj);
    }

    /**
     * set MediaPlayer Listener
     *
     * @param aListener Listener reference.
     */
    public void setOnMediaPlayerNotifyEventListener(OnMediaPlayerNotifyEventListener aListener) {
        mMediaPlayerNotifyEventListener = aListener;
    }

    private OnMediaPlayerNotifyEventListener mMediaPlayerNotifyEventListener;

    /**
     * enable low delay audio effect processing
     *
     * @param enable enable
     */
    @Override
    public void setAudioEffectLowDelay(boolean enable) {
        nativeSetAudioEffectLowDelay(mNativePlayerPara, enable);
    }

    /**
     * set ProxyServer Config parameter
     *
     * @param ip        ip
     * @param port      port
     * @param authenkey authenkey
     * @param useProxy  useProxy
     */
    @Override
    public void setProxyServerConfig(String ip, int port, String authenkey, boolean useProxy) {
        nativeCongfigProxyServer(mNativePlayerPara, ip, port, authenkey, useProxy);
    }
}
