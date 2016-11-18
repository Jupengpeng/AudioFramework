package com.yinchao.android.media.service;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.media.MediaBrowserCompat;
import android.support.v4.media.MediaBrowserServiceCompat;
import android.support.v4.media.session.MediaButtonReceiver;
import android.support.v4.media.session.MediaSessionCompat;
import android.support.v4.media.session.PlaybackStateCompat;
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.List;

/**
 * Created by Administrator on 2016/11/15.
 */

public class YCMusicPlayService extends MediaBrowserServiceCompat
        implements MusicPlayManager.PlayServiceCallback {

    private static String TAG = "YCMusicPlayerService";


    // Extra on MediaSession that contains the Cast device name currently connected to
    public static final String EXTRA_CONNECTED_CAST = "com.example.android.uamp.CAST_NAME";
    // The action of the incoming Intent indicating that it contains a command
    // to be executed (see {@link #onStartCommand})
    public static final String ACTION_CMD = "com.example.android.uamp.ACTION_CMD";
    // The key in the extras of the incoming Intent indicating the command that
    // should be executed (see {@link #onStartCommand})
    public static final String CMD_NAME = "CMD_NAME";
    // A value of a CMD_NAME key in the extras of the incoming Intent that
    // indicates that the music playback should be paused (see {@link #onStartCommand})
    public static final String CMD_PAUSE = "CMD_PAUSE";
    // A value of a CMD_NAME key that indicates that the music playback should switch
    // to local playback from cast playback.
    public static final String CMD_STOP_CASTING = "CMD_STOP_CASTING";
    // Delay stopSelf by using a handler.
    private static final int STOP_DELAY = 30000;

    private MediaSessionCompat mSession;
    private DelayedStopHandler mDelayedStopHandler = new DelayedStopHandler(this);

    private MusicPlayManager mMusicPlayManager;


    /**
     * @see YCMusicPlayService
     */
    @Override
    public void onCreate() {
        super.onCreate();
        mSession = new MediaSessionCompat(this, "YCMusicPlayerService");

        mMusicPlayManager = new MusicPlayManager(this);

        setSessionToken(mSession.getSessionToken());
        mSession.setCallback(mMusicPlayManager.getMediaSessionCallback());
        mSession.setFlags(MediaSessionCompat.FLAG_HANDLES_MEDIA_BUTTONS |
                MediaSessionCompat.FLAG_HANDLES_TRANSPORT_CONTROLS);



        mDelayedStopHandler.removeCallbacksAndMessages(null);
        mDelayedStopHandler.sendEmptyMessageDelayed(0,STOP_DELAY);

    }

    HandlerThread handerThread;
    Handler hanlder;

    @Override
    public void onDestroy() {
        super.onDestroy();


    }


    /**
     * 控制Service重启，处理重启的一些事件.
     *
     * @param intent
     * @param flags
     * @param startId
     * @return
     */
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null) {
            MediaButtonReceiver.handleIntent(mSession, intent);
        }
        mDelayedStopHandler.removeCallbacksAndMessages(null);
        mDelayedStopHandler.sendEmptyMessageDelayed(0,STOP_DELAY);
        return START_STICKY;
    }


    @Nullable
    @Override
    public BrowserRoot onGetRoot(@NonNull String clientPackageName, int clientUid, @Nullable Bundle rootHints) {
        Log.d(TAG, "OnGetRoot: clientPackageName=" + clientPackageName + "; clientUid=" + clientUid + " ; rootHints=" + rootHints);
        PlaybackStateCompat.Builder builder = new PlaybackStateCompat.Builder();
        builder.setState(PlaybackStateCompat.STATE_NONE, 0, 0);
        mSession.setPlaybackState(builder.build());

        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                while (true){

                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        };

        Thread thread = new Thread(runnable);
        thread.start();

        return new BrowserRoot("MEDIA_ID_ROOT", null);
    }

    @Override
    public void onLoadChildren(@NonNull String parentId, @NonNull Result<List<MediaBrowserCompat.MediaItem>> result) {

    }


    @Override
    public void onPlaybackStart() {
        if (!mSession.isActive()) {
            mSession.setActive(true);
        }

        mDelayedStopHandler.removeCallbacksAndMessages(null);

        startService(new Intent(getApplicationContext(), YCMusicPlayService.class));
    }

    @Override
    public void onNotificationRequired() {

    }

    @Override
    public void onPlaybackStop() {
        mDelayedStopHandler.removeCallbacksAndMessages(null);
        mDelayedStopHandler.sendEmptyMessageDelayed(0, STOP_DELAY);
        stopForeground(true);
    }



    @Override
    public IBinder onBind(Intent intent) {
        return super.onBind(intent);
    }

    @Override
    public void onRebind(Intent intent) {
        super.onRebind(intent);
    }

    @Override
    public void onPlaybackStateUpdated(PlaybackStateCompat newState) {

        mSession.setPlaybackState(newState);
    }

    /**
     * A simple handler that stops the service if playback is not active (playing)
     */
    private static class DelayedStopHandler extends Handler {
        private final WeakReference<YCMusicPlayService> mWeakReference;

        private DelayedStopHandler(YCMusicPlayService service) {
            mWeakReference = new WeakReference<>(service);
        }

        @Override
        public void handleMessage(Message msg) {
            YCMusicPlayService service = mWeakReference.get();
//            service.stopSelf();
        }
    }
}
