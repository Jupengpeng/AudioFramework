package com.example.playerdemo;

import com.android.ychao.media.player.*;
public interface OnStateChangeListener {
    /**
     * 锟斤拷锟斤拷准锟斤拷锟缴癸拷
     */
    void onPrepared(int error, int av);

    /**
     * 锟斤拷锟脚匡拷始
     */
    void onStarted();

    /**
     * 锟斤拷锟斤拷锟斤拷停
     */
    void onPaused();

    /**
     * 锟斤拷锟斤拷锟斤拷锟�
     */
    void onCompleted();
    
    void onSeekCompleted();

    /**
     * 鎾斁鍑洪敊
     *
     * @param error            error
     * @param httpCode         http鐘舵�佺爜
     * @param notificationInfo 涓婃姤淇℃伅
     */
    void onError(int error, int httpCode, MediaPlayerNotificationInfo notificationInfo);

    /**
     * 锟斤拷始锟斤拷锟斤拷
     */
    void onBufferingStarted();

    /**
     * 锟斤拷锟斤拷锟斤拷锟�
     */
    void onBufferingDone();

    /**
     * 锟斤拷锟斤拷锟斤拷锟�
     */
    void onBufferFinished();

}
