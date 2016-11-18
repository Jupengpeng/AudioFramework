package com.yinchao.android.media.service;

import android.content.Intent;
import android.os.Bundle;
import android.os.ResultReceiver;
import android.support.annotation.NonNull;
import android.support.v4.media.session.MediaSessionCompat;
import android.support.v4.media.session.PlaybackStateCompat;

import com.android.ychao.media.player.MediaPlayerNotificationInfo;
import com.android.ychao.media.proxy.MediaPlayerProxy;
import com.android.ychao.media.proxy.OnStateChangeListener;
import com.android.ychao.media.proxy.OnStateChangeListenerAdapter;

/**
 * Created by Administrator on 2016/11/16.
 * <p>
 * MusicPlayManager.
 */
public class MusicPlayManager {

    private MediaSessionCallback mMediaSessionCallback;

    private MediaPlayerProxy mMediaPlayerProxy;
    private PlayServiceCallback mPlayServiceCallback;

    String pluginPath;
    String songPath;


    /**
     * @param playServiceCallback
     */
    public MusicPlayManager(PlayServiceCallback playServiceCallback) {

        mPlayServiceCallback = playServiceCallback;

        pluginPath = "/data/data/com.yinchao.android.app/lib";
        songPath = "http://audio.yinchao.cn/accompaniment/2016071500000000001.mp3";

        mMediaSessionCallback = new MediaSessionCallback();
        mMediaPlayerProxy = createMediaPlayerProxy();
    }

    /**
     * getMediaSessionCallback.
     *
     * @return
     */
    public MediaSessionCompat.Callback getMediaSessionCallback() {
        return mMediaSessionCallback;
    }

    /**
     * release.
     */
    public void release() {
        if (mMediaPlayerProxy != null) {
            mMediaPlayerProxy.release();
            mMediaPlayerProxy = null;
        }
    }


    public MediaPlayerProxy createMediaPlayerProxy() {

        MediaPlayerProxy playerProxy = new MediaPlayerProxy(pluginPath);
        playerProxy.setOnStateChangeListener(mOnStateChangeListener);
        return playerProxy;
    }


    public void handlePlayRequest() {
        try {
            mMediaPlayerProxy.palySong(songPath);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Handle a request to pause music
     */
    public void handlePauseRequest() {

        mMediaPlayerProxy.pause();
    }

    /**
     * Handle a request to stop music
     *
     * @param withError Error message in case the stop has an unexpected cause. The error
     *                  message will be set in the PlaybackState and will be visible to
     *                  MediaController clients.
     */
    public void handleStopRequest(String withError) {
        mMediaPlayerProxy.stop();
    }


    /**
     * MediaSessionCallback.
     */
    private class MediaSessionCallback extends MediaSessionCompat.Callback {
        @Override
        public void onPlay() {
//            LogHelper.d(TAG, "play");
//            if (mQueueManager.getCurrentMusic() == null) {
//                mQueueManager.setRandomQueue();
//            }
            handlePlayRequest();
        }

        @Override
        public void onSkipToQueueItem(long queueId) {
//            LogHelper.d(TAG, "OnSkipToQueueItem:" + queueId);
//            mQueueManager.setCurrentQueueItem(queueId);
//            handlePlayRequest();
//            mQueueManager.updateMetadata();
        }

        @Override
        public void onSeekTo(long position) {
            mMediaPlayerProxy.seek((int) position);
        }

        @Override
        public void onPlayFromMediaId(String mediaId, Bundle extras) {
        }

        @Override
        public void onPause() {
            handlePauseRequest();
        }

        @Override
        public void onStop() {
            handleStopRequest(null);
        }

        @Override
        public void onSkipToNext() {
        }

        @Override
        public void onSkipToPrevious() {
        }

        @Override
        public void onCustomAction(@NonNull String action, Bundle extras) {
            if (MediaCustomAction.ACTION_VOLUME.equals(action)) {
                float left = MediaCustomAction.castVolumeLeft(extras) / 100.0f;
                float right = MediaCustomAction.castVolumeRight(extras) / 100.0f;
                mMediaPlayerProxy.setVolume(left, right);
            }
        }

        @Override
        public void onPlayFromSearch(final String query, final Bundle extras) {

        }

        @Override
        public void onCommand(String command, Bundle extras, ResultReceiver cb) {
            super.onCommand(command, extras, cb);
        }

        @Override
        public boolean onMediaButtonEvent(Intent mediaButtonEvent) {
            return super.onMediaButtonEvent(mediaButtonEvent);
        }
    }

    PlaybackStateCompat.Builder builder = new PlaybackStateCompat.Builder();

    /**
     * OnStateChangeListener.
     */
    private OnStateChangeListener mOnStateChangeListener = new OnStateChangeListenerAdapter() {
        @Override
        public void onPrepared(int error, int av) {
            mMediaPlayerProxy.start();
        }

        @Override
        public void onStarted() {
            builder.setActions(PlaybackStateCompat.ACTION_PLAY);
            builder.setState(PlaybackStateCompat.STATE_PLAYING, 0, 0);
            mPlayServiceCallback.onPlaybackStateUpdated(builder.build());
        }

        @Override
        public void onPaused() {
            int pos = mMediaPlayerProxy.getPosition();
            builder.setActions(PlaybackStateCompat.ACTION_PAUSE);
            builder.setState(PlaybackStateCompat.STATE_PAUSED, pos, 0);
            mPlayServiceCallback.onPlaybackStateUpdated(builder.build());
        }

        @Override
        public void onCompleted() {
            super.onCompleted();
        }

        @Override
        public void onSeekCompleted() {
            super.onSeekCompleted();
        }

        @Override
        public void onError(int error, int httpCode, MediaPlayerNotificationInfo notificationInfo) {
            super.onError(error, httpCode, notificationInfo);
        }

        @Override
        public void onOver(int id) {
            super.onOver(id);
        }

        @Override
        public void onBufferingStarted() {
            super.onBufferingStarted();
        }

        @Override
        public void onBufferingDone() {
            super.onBufferingDone();
        }

        @Override
        public void onBufferFinished() {
            super.onBufferFinished();
        }
    };

    /**
     * PlayServiceCallback.
     */
    public interface PlayServiceCallback {
        void onPlaybackStart();

        void onNotificationRequired();

        void onPlaybackStop();

        void onPlaybackStateUpdated(PlaybackStateCompat newState);
    }

}
