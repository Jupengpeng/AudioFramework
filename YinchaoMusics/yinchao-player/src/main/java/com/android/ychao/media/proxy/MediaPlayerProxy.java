package com.android.ychao.media.proxy;

import android.util.Log;

import com.android.ychao.media.player.IMediaPlayer;
import com.android.ychao.media.player.MediaPlayerNotificationInfo;
import com.android.ychao.media.player.Normalizer;
import com.android.ychao.media.player.YCMediaPlayer;

public final class MediaPlayerProxy {

    public static final int HUNDRED = 100;

    private final int K_FREQUENCY_NUMBER = 512;

    private short[] mFreqBuffer = new short[K_FREQUENCY_NUMBER];
    /**
     * 起始状态.
     */
    private PlayStatus mPlayStatus = PlayStatus.STATUS_STOPPED;


    private volatile YCMediaPlayer mTTMediaPlayer;
    private IMediaPlayer mCurMediaPlayer;

    private static final String TAG = "MediaPlayerProxy";


    private OnMediaDurationUpdateListener mMediaDurationUpdateListener = null;

    private OnStateChangeListener mStateChangeListener = null;

    /**
     * @param pluginPath
     */
    public MediaPlayerProxy(String pluginPath) {
        mCurMediaPlayer = buildTTMediaPlayer(pluginPath);
    }

    public void play(String sourcePath) throws Exception {
        if (mCurMediaPlayer != null)
            mCurMediaPlayer.setDataSourceAsync(sourcePath, YCMediaPlayer.OPEN_DEFAULT);
    }

    public void play(String sourcePath, String cacheFilePath) throws Exception {
        if (mCurMediaPlayer != null) {
            mCurMediaPlayer.setCacheFilePath(cacheFilePath);
            mCurMediaPlayer.setDataSourceAsync(sourcePath, YCMediaPlayer.OPEN_DEFAULT);
        }
    }

    /**
     * start.
     */
    public void start() {
        mCurMediaPlayer.play();
        mPlayStatus = PlayStatus.STATUS_PLAYING;
    }

    /**
     * pause.
     */
    public void pause() {
        if (mCurMediaPlayer != null)
            mCurMediaPlayer.pause();
        mPlayStatus = PlayStatus.STATUS_PAUSED;
    }

    /**
     * resume.
     */
    public void resume() {
        mCurMediaPlayer.resume();
        mPlayStatus = PlayStatus.STATUS_PLAYING;
    }

    /**
     * stop.
     */
    public void stop() {
        if (mCurMediaPlayer != null)
            mCurMediaPlayer.stop();
        mPlayStatus = PlayStatus.STATUS_STOPPED;
    }


    /**
     * 播放音频文件.
     *
     * @param sourcePath
     * @throws Exception
     */
    public void palySong(String sourcePath) throws Exception {
//        stop();
        play(sourcePath);
        mPlayStatus = PlayStatus.STATUS_STOPPED;
    }

    /**
     * 播放音频并设置音频缓存文件.
     *
     * @param sourcePath
     * @param cacheFilePath
     * @throws Exception
     */
    public void palySong(String sourcePath, String cacheFilePath) throws Exception {
        stop();
        play(sourcePath, cacheFilePath);
        mPlayStatus = PlayStatus.STATUS_STOPPED;
    }


    /**
     * 获取播放状态.
     *
     * @return
     */
    public PlayStatus getPlayStatus() {
        return mPlayStatus;
    }


    /**
     * 获取音频文件总时长.
     *
     * @return
     */
    public int duration() {
        return mCurMediaPlayer.duration();
    }

    /**
     * 移动到指定位置播放.
     *
     * @param value
     */
    public void seek(int value) {
        mCurMediaPlayer.setPosition(value, 0);

    }

    /**
     * 当前播放位置.
     *
     * @return
     */
    public int getPosition() {
        if (mCurMediaPlayer != null) {
            return mCurMediaPlayer.getPosition();
        }
        return 0;
    }


    public void setPosition(int position, int flag) {
        mCurMediaPlayer.setPosition(position, flag);
    }

    /**
     * 播放一段音频文件.
     *
     * @param aStart
     * @param aEnd
     */
    public void setPlayRange(int aStart, int aEnd) {
        mCurMediaPlayer.setPlayRange(aStart, aEnd);
    }


    public float getBufferPercent() {
        if (mPlayStatus == PlayStatus.STATUS_PLAYING || mPlayStatus == PlayStatus.STATUS_PAUSED) {
            return mCurMediaPlayer.getBufferPercent();
        } else {
            return 0.0f;
        }
    }

    /**
     * 设置状态改变 Listener.
     *
     * @param listener
     */
    public void setOnStateChangeListener(OnStateChangeListener listener) {
        mStateChangeListener = listener;
        mTTMediaPlayer.setOnMediaPlayerNotifyEventListener(mMediaPlayerNotifyEventListener);
    }

    public void setOnMediaDurationUpdateListen(OnMediaDurationUpdateListener listen) {
        mMediaDurationUpdateListener = listen;
    }

    /**
     * buildTTMediaPlayer.
     *
     * @param pluginPath
     * @return
     */
    private YCMediaPlayer buildTTMediaPlayer(String pluginPath) {
        byte[] headerBytes = null;
        YCMediaPlayer mediaPlayer = new YCMediaPlayer(headerBytes, pluginPath);
        mTTMediaPlayer = mediaPlayer;
        return mediaPlayer;
    }

    private void releaseTTMediaPlayer() {
        if (mTTMediaPlayer != null) {
            mTTMediaPlayer.release();
            mTTMediaPlayer = null;
        }
    }

    public void release() {
        releaseTTMediaPlayer();
        mCurMediaPlayer = null;
    }

    public void setAudioEffectLowDelay(boolean enable) {
        if (mTTMediaPlayer != null) {
            mTTMediaPlayer.setAudioEffectLowDelay(enable);
        }
    }

    /**
     * 处理 YCMediaPlayer 回调信息.
     * <p>
     * YCMediaPlayer.OnMediaPlayerNotifyEventListener.
     */
    private YCMediaPlayer.OnMediaPlayerNotifyEventListener mMediaPlayerNotifyEventListener = new YCMediaPlayer.OnMediaPlayerNotifyEventListener() {
        @Override
        public void onMediaPlayerNotify(int msgId, int arg1, int arg2, Object obj) {
            Log.i(TAG, "MsgId:" + msgId+" arg1:"+arg1 + " arg2:" +arg2 + " obj:" + obj);
            switch (msgId) {
                case YCMediaPlayer.MEDIA_PREPARE:
                    if (mStateChangeListener != null) {
                        mStateChangeListener.onPrepared(arg1, arg2);
                    }
                    break;

                case YCMediaPlayer.MEDIA_PAUSE:
                    mPlayStatus = PlayStatus.STATUS_PAUSED;
                    if (mStateChangeListener != null) {
                        mStateChangeListener.onPaused();
                        mStateChangeListener.onOver(YCMediaPlayer.MEDIA_PAUSE);
                    }
                    break;

                case YCMediaPlayer.MEDIA_PLAY:
                    mPlayStatus = PlayStatus.STATUS_PLAYING;
                    if (mStateChangeListener != null) {
                        mStateChangeListener.onStarted();
                    }
                    break;

                case YCMediaPlayer.MEDIA_COMPLETE:
                    mPlayStatus = PlayStatus.STATUS_STOPPED;
                    if (mStateChangeListener != null) {
                        mStateChangeListener.onCompleted();
                        mStateChangeListener.onOver(YCMediaPlayer.MEDIA_COMPLETE);
                    }
                    break;

                case YCMediaPlayer.MEDIA_SEEK_COMPLETED:
                    if (mStateChangeListener != null) {
                        mStateChangeListener.onSeekCompleted();
                    }
                    break;

                case YCMediaPlayer.MEDIA_EXCEPTION:
                    mPlayStatus = PlayStatus.STATUS_STOPPED;
                    if (mStateChangeListener != null) {
                        mStateChangeListener.onError(arg1, arg2, (MediaPlayerNotificationInfo) obj);
                        mStateChangeListener.onOver(YCMediaPlayer.MEDIA_EXCEPTION);
                    }
                    break;

                case YCMediaPlayer.MEDIA_BUFFERING_START:
                    if (mStateChangeListener != null) {
                        mStateChangeListener.onBufferingStarted();
                    }
                    break;

                case YCMediaPlayer.MEDIA_BUFFERING_DONE:
                    if (mStateChangeListener != null) {
                        mStateChangeListener.onBufferingDone();
                    }
                    break;

                case YCMediaPlayer.MEDIA_CACHE_COMPLETED:
                    if (mStateChangeListener != null) {
                        mStateChangeListener.onBufferFinished();
                    }
                    break;

                case YCMediaPlayer.MEDIA_UPDATE_DURATION:
                    if (mMediaDurationUpdateListener != null) {
                        mMediaDurationUpdateListener.onMediaDurationUpdated(mCurMediaPlayer.duration());
                    }
                    break;

                case YCMediaPlayer.MEDIA_CLOSE:
                    Log.e(TAG, "MEDIA_CLOSE: proxysize " + arg2);
                    break;
                case YCMediaPlayer.MEDIA_START_RECEIVE_DATA:
                    Log.e(TAG, "MEDIA_START_RECEIVE_DATA:");
                    break;
                case YCMediaPlayer.MEDIA_PREFETCH_COMPLETED:
                    Log.e(TAG, "MEDIA_PREFETCH_COMPLETED:");
                    break;
                case YCMediaPlayer.MEDIA_DNS_DONE:
                    Log.e(TAG, "MEDIA_DNS_DONE:");
                    break;
                case YCMediaPlayer.MEDIA_CONNECT_DONE:
                    Log.e(TAG, "MEDIA_CONNECT_DONE:");
                    break;
                case YCMediaPlayer.MEDIA_HTTP_HEADER_RECEIVED:
                    Log.e(TAG, "MEDIA_HTTP_HEADER_RECEIVED:");
                    break;
                default:
                    Log.e(TAG, "default: unKnow msgId:"+msgId);
            }
        }
    };

    public boolean getFreq(int[] freq, int length) {
        if (freq.length < length) {
            return false;
        }

        if (0 == mCurMediaPlayer.getCurFreq(mFreqBuffer, K_FREQUENCY_NUMBER)) {
            int nNormalize = Normalizer.normalizeFreqBin(freq, length, mFreqBuffer, K_FREQUENCY_NUMBER);
            return 0 == nNormalize;
        }

        return false;
    }

    public boolean getWave(short[] wave, int length) {
        if (wave.length < length) {
            return false;
        }
        return 0 == mCurMediaPlayer.getCurWave(wave, length);
    }


    public void setVolume(float leftVolume, float rightVolume) {
        mCurMediaPlayer.setVolume(leftVolume, rightVolume);
    }

    /**
     * 歌曲时长信息回调.
     */
    public interface OnMediaDurationUpdateListener {
        void onMediaDurationUpdated(int duration);
    }

    /**
     * 播放器网络信息回调.
     */
    public interface OnMediaNetworkListener {
        void onNetwork(int id);
    }


}
