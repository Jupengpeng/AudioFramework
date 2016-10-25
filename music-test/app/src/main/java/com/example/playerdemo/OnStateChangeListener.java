package com.example.playerdemo;

import com.android.ychao.media.player.*;
public interface OnStateChangeListener {
    void onPrepared(int error, int av);

    void onStarted();

    void onPaused();

    void onCompleted();
    
    void onSeekCompleted();
    void onError(int error, int httpCode, MediaPlayerNotificationInfo notificationInfo);
    void onBufferingStarted();
    void onBufferingDone();
    void onBufferFinished();

}
