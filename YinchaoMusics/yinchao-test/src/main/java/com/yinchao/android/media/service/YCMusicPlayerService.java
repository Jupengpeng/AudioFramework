package com.yinchao.android.media.service;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.ResultReceiver;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.media.MediaBrowserCompat;
import android.support.v4.media.MediaBrowserServiceCompat;
import android.support.v4.media.RatingCompat;
import android.support.v4.media.session.MediaButtonReceiver;
import android.support.v4.media.session.MediaSessionCompat;
import android.support.v4.media.session.PlaybackStateCompat;
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.List;

/**
 * Created by Administrator on 2016/11/15.
 */

public class YCMusicPlayerService extends MediaBrowserServiceCompat {

    private static String TAG = "YCMusicPlayerService";

    private MediaSessionCompat mSession;

    @Override
    public void onCreate() {
        super.onCreate();
        mSession = new MediaSessionCompat(this, "YCMusicPlayerService");

        setSessionToken(mSession.getSessionToken());
        mSession.setCallback(new MediaSessionCallback());
        mSession.setFlags(MediaSessionCompat.FLAG_HANDLES_MEDIA_BUTTONS |
                MediaSessionCompat.FLAG_HANDLES_TRANSPORT_CONTROLS);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    /**
     * 控制Service重启，处理重启的一些事件.
     * @param intent
     * @param flags
     * @param startId
     * @return
     */
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null){
            MediaButtonReceiver.handleIntent(mSession, intent);
        }

        return START_STICKY;
    }


    @Nullable
    @Override
    public BrowserRoot onGetRoot(@NonNull String clientPackageName, int clientUid, @Nullable Bundle rootHints) {
        Log.d(TAG, "OnGetRoot: clientPackageName=" + clientPackageName + "; clientUid=" + clientUid + " ; rootHints=" + rootHints);
        return new BrowserRoot("MEDIA_ID_ROOT", null);
    }

    @Override
    public void onLoadChildren(@NonNull String parentId, @NonNull Result<List<MediaBrowserCompat.MediaItem>> result) {

    }

    private class MediaSessionCallback extends MediaSessionCompat.Callback {
        public MediaSessionCallback() {
            super();
        }

        @Override
        public void onCommand(String command, Bundle extras, ResultReceiver cb) {
            super.onCommand(command, extras, cb);
            PlaybackStateCompat.Builder builder= new PlaybackStateCompat.Builder();
            mSession.setPlaybackState(builder.build());
        }

        @Override
        public boolean onMediaButtonEvent(Intent mediaButtonEvent) {
            return super.onMediaButtonEvent(mediaButtonEvent);
        }

        @Override
        public void onPrepare() {
            super.onPrepare();
        }

        @Override
        public void onPrepareFromMediaId(String mediaId, Bundle extras) {
            super.onPrepareFromMediaId(mediaId, extras);
        }

        @Override
        public void onPrepareFromSearch(String query, Bundle extras) {
            super.onPrepareFromSearch(query, extras);
        }

        @Override
        public void onPrepareFromUri(Uri uri, Bundle extras) {
            super.onPrepareFromUri(uri, extras);
        }

        @Override
        public void onPlay() {
            super.onPlay();
        }

        @Override
        public void onPlayFromMediaId(String mediaId, Bundle extras) {
            super.onPlayFromMediaId(mediaId, extras);
        }

        @Override
        public void onPlayFromSearch(String query, Bundle extras) {
            super.onPlayFromSearch(query, extras);
        }

        @Override
        public void onPlayFromUri(Uri uri, Bundle extras) {
            super.onPlayFromUri(uri, extras);
        }

        @Override
        public void onSkipToQueueItem(long id) {
            super.onSkipToQueueItem(id);
        }

        @Override
        public void onPause() {
            super.onPause();
        }

        @Override
        public void onSkipToNext() {
            super.onSkipToNext();
        }

        @Override
        public void onSkipToPrevious() {
            super.onSkipToPrevious();
        }

        @Override
        public void onFastForward() {
            super.onFastForward();
        }

        @Override
        public void onRewind() {
            super.onRewind();
        }

        @Override
        public void onStop() {
            super.onStop();
        }

        @Override
        public void onSeekTo(long pos) {
            super.onSeekTo(pos);
        }

        @Override
        public void onSetRating(RatingCompat rating) {
            super.onSetRating(rating);
        }

        @Override
        public void onCustomAction(String action, Bundle extras) {
            super.onCustomAction(action, extras);
        }
    }


    /**
     * A simple handler that stops the service if playback is not active (playing)
     */
    private static class DelayedStopHandler extends Handler {
        private final WeakReference<YCMusicPlayerService> mWeakReference;

        private DelayedStopHandler(YCMusicPlayerService service) {
            mWeakReference = new WeakReference<>(service);
        }

        @Override
        public void handleMessage(Message msg) {
            YCMusicPlayerService service = mWeakReference.get();
//            if (service != null && service.mPlaybackManager.getPlayback() != null) {
//                if (service.mPlaybackManager.getPlayback().isPlaying()) {
//                    LogHelper.d(TAG, "Ignoring delayed stop since the media player is in use.");
//                    return;
//                }
//                LogHelper.d(TAG, "Stopping service with delay handler.");
                service.stopSelf();
//            }
        }
    }
}
