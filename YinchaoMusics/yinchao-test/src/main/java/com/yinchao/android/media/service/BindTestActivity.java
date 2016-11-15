package com.yinchao.android.media.service;

import android.content.ComponentName;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ResultReceiver;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.media.MediaBrowserCompat;
import android.support.v4.media.MediaMetadataCompat;
import android.support.v4.media.session.MediaControllerCompat;
import android.support.v4.media.session.MediaSessionCompat;
import android.support.v4.media.session.PlaybackStateCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.yinchao.android.media.R;
import com.yinchao.android.media.test.utils.ScheduleTask;

/**
 * Created by Administrator on 2016/11/15.
 */

public class BindTestActivity extends AppCompatActivity {



    TextView console;

    Button button1;
    Button button2;
    Button button3;

    SeekBar seekBarMain;
    SeekBar seekBarLeft;
    SeekBar seekBarRight;

    TextView nowTime;
    TextView totalTime;

    TextView leftNumber;
    TextView rightNumber;


    ScheduleTask scheduleTask = new ScheduleTask();

    private MediaBrowserCompat mMediaBrowser;



    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mediaplayer_test);

        bindView();

        initLisener();

        mMediaBrowser = new MediaBrowserCompat(this, new ComponentName(this, YCMusicPlayerService.class), mConnectionCallback, null);
    }



    /**
     *
     */
    private void bindView() {

        console = (TextView) findViewById(R.id.console);

        button1 = (Button) findViewById(R.id.start);
        button2 = (Button) findViewById(R.id.cancel);
        button3 = (Button) findViewById(R.id.cancel2);

        seekBarMain = (SeekBar) findViewById(R.id.seekBar_progress);
        seekBarLeft = (SeekBar) findViewById(R.id.seekBar_left);
        seekBarRight = (SeekBar) findViewById(R.id.seekBar_right);

        nowTime = (TextView) findViewById(R.id.now_time);
        totalTime = (TextView) findViewById(R.id.total_time);
        leftNumber = (TextView) findViewById(R.id.left_number);
        rightNumber = (TextView) findViewById(R.id.right_number);
    }



    /**
     *
     */
    private void initLisener() {
        button1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                getSupportMediaController().getTransportControls().play();
            }
        });

        button2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MediaMetadataCompat metadata = getSupportMediaController().getMetadata();
                metadata.keySet();
            }
        });
        button3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                getSupportMediaController().sendCommand("test",null,resultReceiver);
            }
        });


    }
    ResultReceiver resultReceiver = new ResultReceiver(new Handler()){
        @Override
        protected void onReceiveResult(int resultCode, Bundle resultData) {
            super.onReceiveResult(resultCode, resultData);
        }

        @Override
        public void send(int resultCode, Bundle resultData) {
            super.send(resultCode, resultData);
        }
    };



    @Override
    protected void onStart() {
        super.onStart();

        mMediaBrowser.connect();
    }

    @Override
    protected void onStop() {
        super.onStop();mMediaBrowser.disconnect();
    }

    private void connectToSession(MediaSessionCompat.Token token) throws RemoteException {
        MediaControllerCompat mediaController = new MediaControllerCompat(this, token);
        setSupportMediaController(mediaController);
        mediaController.registerCallback(mMediaControllerCallback);


//        mediaController.

    }


    // Callback that ensures that we are showing the controls
    private final MediaControllerCompat.Callback mMediaControllerCallback =
            new MediaControllerCompat.Callback() {
                @Override
                public void onPlaybackStateChanged(@NonNull PlaybackStateCompat state) {
//                    if (shouldShowControls()) {
//                        showPlaybackControls();
//                    } else {
//                        LogHelper.d(TAG, "mediaControllerCallback.onPlaybackStateChanged: " +
//                                "hiding controls because state is ", state.getState());
//                        hidePlaybackControls();
//                    }
                }

                @Override
                public void onMetadataChanged(MediaMetadataCompat metadata) {
//                    if (shouldShowControls()) {
//                        showPlaybackControls();
//                    } else {
//                        LogHelper.d(TAG, "mediaControllerCallback.onMetadataChanged: " +
//                                "hiding controls because metadata is null");
//                        hidePlaybackControls();
//                    }
                }
            };

    private final MediaBrowserCompat.ConnectionCallback mConnectionCallback =
            new MediaBrowserCompat.ConnectionCallback() {
                @Override
                public void onConnected() {
//                    LogHelper.d(TAG, "onConnected");
                    try {
                        connectToSession(mMediaBrowser.getSessionToken());
                    } catch (RemoteException e) {
//                        LogHelper.e(TAG, e, "could not connect media controller");
//                        hidePlaybackControls();
                    }
                }
            };


}
