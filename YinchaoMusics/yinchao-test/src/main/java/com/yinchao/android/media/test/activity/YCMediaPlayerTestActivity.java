package com.yinchao.android.media.test.activity;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import com.android.ychao.media.player.MediaPlayerNotificationInfo;
import com.android.ychao.media.proxy.MediaPlayerProxy;
import com.android.ychao.media.proxy.OnStateChangeListenerAdapter;
import com.android.ychao.media.proxy.PlayStatus;
import com.yinchao.android.media.R;
import com.yinchao.android.media.service.BindTestActivity;
import com.yinchao.android.media.test.utils.ScheduleTask;
import com.yinchao.android.media.test.utils.TimeFormatUtils;

/**
 * Created by Administrator on 2016/11/14.
 */

public class YCMediaPlayerTestActivity extends AppCompatActivity {

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

    private int number = 0;

    int left = 0;
    int right = 0;

    String pluginPath;
    String songPath;

    MediaPlayerProxy mediaPlayerProxy;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mediaplayer_test);

        pluginPath = "/data/data/com.yinchao.android.app/lib";
        songPath = "http://audio.yinchao.cn/accompaniment/2016071500000000001.mp3";

        bindView();

        initLisener();
        initSeekBar();

        initMediaPlayer();
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


    private void initSeekBar() {

        seekBarLeft.setMax(100);
        seekBarRight.setMax(100);
        /**
         *
         */
        seekBarMain.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int progressx;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressx = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mediaPlayerProxy.seek(progressx);
            }
        });
        seekBarLeft.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                left = progress;
                setVolume(left, right);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                leftNumber.setText(left+"%");
            }
        });
        seekBarRight.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                right = progress;
                setVolume(left, right);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                rightNumber.setText(right+"%");
            }
        });

    }


    private void setVolume(float leftx, float rightx) {
        mediaPlayerProxy.setVolume(leftx/100, rightx/100);
        Log.i("setVolume","left:"+leftx+"right:"+rightx);
    }

    /**
     *
     */
    private void initLisener() {
        button1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                try {
                    mediaPlayerProxy.palySong(songPath);
                } catch (Exception e){
                    e.printStackTrace();
                }
            }
        });

        button2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mediaPlayerProxy.getPlayStatus() == PlayStatus.STATUS_PLAYING){

                    mediaPlayerProxy.pause();
                } else {
                    mediaPlayerProxy.resume();
                }
            }
        });
        button3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                mediaPlayerProxy.stop();

                start();
            }
        });


    }

    private void start(){
        startActivity(new Intent(this, BindTestActivity.class));
    }


    private void initMediaPlayer() {


        mediaPlayerProxy = new MediaPlayerProxy(pluginPath);

        mediaPlayerProxy.setOnStateChangeListener(new OnStateChangeListenerAdapter() {

            @Override
            public void onPrepared(int error, int av) {
                super.onPrepared(error, av);
                mediaPlayerProxy.start();
            }

            @Override
            public void onStarted() {
                super.onStarted();
                scheduleTask.schedule(new ScheduleTask.RunTask() {
                    @Override
                    public void run() {
                        int all = mediaPlayerProxy.duration();
                        int now = mediaPlayerProxy.getPosition();
                        seekBarMain.setMax(all);
                        seekBarMain.setProgress(now);

                        nowTime.setText(TimeFormatUtils.format(now/1000));
                        totalTime.setText(TimeFormatUtils.format(all/1000));
                        Log.e("scheduleTask",":"+now+":"+all);
                    }
                }, 200, 500);

            }

            @Override
            public void onPaused() {
                super.onPaused();
            }

            @Override
            public void onCompleted() {
                super.onCompleted();
            }

            @Override
            public void onOver(int id) {
                super.onOver(id);
                scheduleTask.cancel();
            }

            @Override
            public void onSeekCompleted() {
                super.onSeekCompleted();
                mediaPlayerProxy.start();
            }

            @Override
            public void onError(int error, int httpCode, MediaPlayerNotificationInfo notificationInfo) {
                super.onError(error, httpCode, notificationInfo);
                console.setText("error:" + error);

            }
        });


    }

}
