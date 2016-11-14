package com.android.ychao.media.proxy;

import com.android.ychao.media.player.MediaPlayerNotificationInfo;

/**
 * Created by Administrator on 2016/11/14.
 *
 * 使用 Adapter class 可以只写自己关注的回调方法.
 */

public class OnStateChangeListenerAdapter implements OnStateChangeListener {


    @Override
    public void onPrepared(int error, int av) {

    }

    @Override
    public void onStarted() {

    }

    @Override
    public void onPaused() {

    }

    @Override
    public void onCompleted() {

    }

    @Override
    public void onSeekCompleted() {

    }

    @Override
    public void onError(int error, int httpCode, MediaPlayerNotificationInfo notificationInfo) {

    }

    @Override
    public void onOver(int id) {

    }

    @Override
    public void onBufferingStarted() {

    }

    @Override
    public void onBufferingDone() {

    }

    @Override
    public void onBufferFinished() {

    }
}
