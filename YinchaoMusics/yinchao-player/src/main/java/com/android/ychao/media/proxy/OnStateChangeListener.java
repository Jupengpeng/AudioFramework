package com.android.ychao.media.proxy;


import com.android.ychao.media.player.MediaPlayerNotificationInfo;

/**
 * 播放器状态回调.
 */


public interface OnStateChangeListener {
    /**
     * 播放器准备就绪.
     *
     * @param error
     * @param av
     */
    void onPrepared(int error, int av);

    /**
     * 开始播放.
     */
    void onStarted();

    /**
     * 暂停播放.
     */
    void onPaused();

    /**
     * 播放完成.
     */
    void onCompleted();

    /**
     * seek() 完成.
     */
    void onSeekCompleted();

    /**
     * 播放器异常，异常信息.
     *
     * @param error
     * @param httpCode
     * @param notificationInfo
     */
    void onError(int error, int httpCode, MediaPlayerNotificationInfo notificationInfo);


    void onOver(int id);

    /**
     * buffer started.
     */
    void onBufferingStarted();

    /**
     * buffer done.
     */
    void onBufferingDone();

    /**
     * buffer finished.
     */
    void onBufferFinished();

}
