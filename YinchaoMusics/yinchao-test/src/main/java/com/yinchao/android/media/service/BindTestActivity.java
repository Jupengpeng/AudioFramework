package com.yinchao.android.media.service;

import android.content.ComponentName;
import android.os.Bundle;
import android.os.RemoteException;
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
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import com.yinchao.android.media.R;
import com.yinchao.android.media.test.utils.ScheduleTask;

/**
 * Created by Administrator on 2016/11/15.
 *
 *
 *
 */
public class BindTestActivity extends AppCompatActivity {


    TextView console;

    Button button1;
    Button button2;
    Button button3;

    ImageView skipPrevious;
    ImageView skipNext;
    ImageView play;

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

        setTitle("BindTestActivity");
        bindView();

        initLisener();
        initSeekBar();
        mMediaBrowser = new MediaBrowserCompat(this, new ComponentName(this, YCMusicPlayService.class), mConnectionCallback, null);
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


        skipPrevious = (ImageView) findViewById(R.id.skip_previous);
        play = (ImageView) findViewById(R.id.play);
        skipNext = (ImageView) findViewById(R.id.skip_next);
    }


    /**
     *
     */
    private void initLisener() {

        skipPrevious.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                getSupportMediaController().getTransportControls().skipToPrevious();
//                getSupportMediaController().setVolumeTo(10,1);
            }
        });

        skipNext.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                getSupportMediaController().getTransportControls().skipToNext();
            }
        });

        play.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (lastState.getState() == PlaybackStateCompat.STATE_PLAYING) {
                    getSupportMediaController().getTransportControls().pause();
                    play.setImageResource(R.drawable.uamp_ic_play_arrow_white_48dp);
                } else {
                    getSupportMediaController().getTransportControls().play();
                    play.setImageResource(R.drawable.uamp_ic_pause_white_48dp);
                }
            }
        });

    }


    private void initSeekBar() {

        seekBarMain.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        seekBarLeft.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                vleft = progress;
                setVolume(vleft, vright);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
        seekBarRight.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                vright = progress;
                setVolume(vleft, vright);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });


    }

    int vleft = 100;
    int vright = 100;

    private void setVolume(int left, int right) {
        Bundle bundle = MediaCustomAction.buildByVolume(left, right);
        getSupportMediaController()
                .getTransportControls()
                .sendCustomAction(MediaCustomAction.ACTION_VOLUME, bundle);
    }


    @Override
    protected void onStart() {
        super.onStart();
        if (!mMediaBrowser.isConnected()){
            mMediaBrowser.connect();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        mMediaBrowser.disconnect();
        if (getSupportMediaController() != null) {
            getSupportMediaController().unregisterCallback(mMediaControllerCallback);
        }
    }



    private void connectToSession(MediaSessionCompat.Token token) throws RemoteException {
        MediaControllerCompat mediaController = new MediaControllerCompat(this, token);
        setSupportMediaController(mediaController);
        mediaController.registerCallback(mMediaControllerCallback);

        lastState = getSupportMediaController().getPlaybackState();

//        getSupportMediaController().setVolumeTo(100,1);

    }

    PlaybackStateCompat lastState;

    // Callback that ensures that we are showing the controls
    private final MediaControllerCompat.Callback mMediaControllerCallback =
            new MediaControllerCompat.Callback() {
                @Override
                public void onPlaybackStateChanged(@NonNull PlaybackStateCompat state) {
                    lastState = state;

                    if (state.getState() == PlaybackStateCompat.STATE_PLAYING) {
                        play.setImageResource(R.drawable.uamp_ic_pause_white_48dp);
                    } else if (state.getState() == PlaybackStateCompat.STATE_PAUSED) {
                        play.setImageResource(R.drawable.uamp_ic_play_arrow_white_48dp);
                    }
                }

                @Override
                public void onMetadataChanged(MediaMetadataCompat metadata) {


                }
            };

    private final MediaBrowserCompat.ConnectionCallback mConnectionCallback =
            new MediaBrowserCompat.ConnectionCallback() {
                @Override
                public void onConnected() {

                    try {
                        connectToSession(mMediaBrowser.getSessionToken());
                    } catch (RemoteException e) {
                    }
                }
            };


}
