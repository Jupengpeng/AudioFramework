package com.example.playerdemo;

import android.app.Activity;
import android.content.res.Configuration;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import com.android.ychao.media.player.MediaPlayerNotificationInfo;
import com.example.playerdemo.utils.PermissionUtils;
import com.yinchao.android.app.R;

import java.util.Timer;
import java.util.TimerTask;

public class MainActivity extends Activity {
    private static final String TAG = "@@@Player Example";
    MediaPlayerProxy mPlayProxy = null;
    Button btnPauseResume;
    Button btnSeek;
    String defaulturl;
    String cachePath;
    EditText urlEdit;
    TextView currentTimeTV;
    TextView totalTimeTV;
    SeekBar mSeekBar;
    private static int position;
    Timer timer = new Timer();
    int seekn = 0;
    private boolean mOnlineSrc = false;

    // private TTEqualizer.Settings mEqualizerSetting = null;

    @SuppressWarnings("deprecation")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFormat(PixelFormat.UNKNOWN);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);


        if (PermissionUtils.checkRecordAudioPermission(this)) {

        }
        if (PermissionUtils.checkCallPhonePermission(this)) {

        }
        if (PermissionUtils.checkReadPhoneStatePermission(this)) {

        }
        if (PermissionUtils.checkSdcardPermission(this)) {

        }


        setContentView(R.layout.activity_main_test);


//		String pluginPath = "/data/data/com.firmlyshell.app.music.test/lib";
        String pluginPath = "/data/data/com.firmlyshell.app/lib";
//		String pluginPath = "/data/app/com.firmlyshell.app-1/lib/arm";
        try {

            Log.e(TAG, Environment.getExternalStoragePublicDirectory("cahce").getAbsolutePath());
//            Log.e(TAG, get.getAbsolutePath());
        } catch (Exception re) {

        }

        Log.e(TAG, "onCreate" + position + " mPlayProxy :" + mPlayProxy);
//		timer.schedule(task, 1000, 1000);
        if (mPlayProxy != null) {
            mPlayProxy.release();
            mPlayProxy = null;
        }
        totalTimeTV = (TextView) findViewById(R.id.textView2);
        currentTimeTV = (TextView) findViewById(R.id.textView3);
        mPlayProxy = new MediaPlayerProxy(pluginPath);

        mPlayProxy.setOnStateChangeListener(new OnStateChangeListener() {

            @Override
            public void onStarted() {
                Log.e(TAG, "onStarted ");
            }

            @Override
            public void onSeekCompleted() {
                Log.e(TAG, "onSeekCompleted ");
            }

            @Override
            public void onPrepared(int error, int av) {

                Log.e(TAG, "onPrepared ");

                int nDuration = mPlayProxy.duration();
                Log.e(TAG, "nDuration " + nDuration);
                int sec = nDuration / 1000;
                int min = sec / 60;
                int msec = sec % 60;
                totalTimeTV.setText(min + ":" + msec);
                mSeekBar.setMax(nDuration);
                mPlayProxy.start();


                timer.schedule(task, 1000, 1000);
            }

            @Override
            public void onPaused() {
                Log.e(TAG, "onPaused ");
            }

            @Override
            public void onError(int error, int httpCode,
                                MediaPlayerNotificationInfo notificationInfo) {
                Log.e(TAG, "onError :" + error + " httpCode:" + httpCode + "::" + notificationInfo);
                timer.cancel();

            }

            @Override
            public void onCompleted() {
                Log.e(TAG, "onCompleted ");
                timer.cancel();
            }

            @Override
            public void onBufferingStarted() {
                Log.e(TAG, "onBufferingStarted ");
            }

            @Override
            public void onBufferingDone() {
                Log.e(TAG, "onBufferingDone ");
            }

            @Override
            public void onBufferFinished() {
                Log.e(TAG, "onBufferFinished ");
            }
        });
        btnPauseResume = (Button) findViewById(R.id.button2);

        defaulturl = "http://audio.yinchao.cn/accompaniment/2016071500000000001.mp3";// "/sdcard/background.mp3";
        mOnlineSrc = false;
        cachePath = "/sdcard/audio5.tmp";

        urlEdit = (EditText) findViewById(R.id.editText1);
        urlEdit.setText(defaulturl);

        mSeekBar = (SeekBar) findViewById(R.id.seekBar1);

        mSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                // TODO Auto-generated method stub

            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress,
                                          boolean fromUser) {
                // TODO Auto-generated method stub

            }
        });
        // play
        ((Button) findViewById(R.id.button1))
                .setOnClickListener(new OnClickListener() {
                    public void onClick(View view) {
                        try {
                            String playurl = urlEdit.getText().toString();
                            // mPlayProxy.palySong(playurl, cachePath);
                            mPlayProxy.palySong(playurl);
                            Log.e(TAG, "start player ");
                        } catch (Exception e) {

                        }
                    }
                });

        // pause
        btnPauseResume.setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                try {
                    String str = btnPauseResume.getText().toString();
                    if (str.equals("Pause")) {
                        mPlayProxy.pause();
                        btnPauseResume.setText("Resume");
                    } else {
                        mPlayProxy.resume();
                        btnPauseResume.setText("Pause");
                    }
                } catch (Exception e) {
                }
            }
        });

        // stop
        ((Button) findViewById(R.id.button3))
                .setOnClickListener(new OnClickListener() {
                    public void onClick(View view) {
                        try {
                            int aClear = 1;
                            mPlayProxy.stop();
                            position = 0;
                        } catch (Exception e) {

                        }
                    }
                });


    }

    public void onConfigurationChanged(Configuration newConfig) {

        super.onConfigurationChanged(newConfig);

        if (this.getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE) {
            // 加入横屏要处理的代码
        } else if (this.getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
            // 加入竖屏要处理的代码
        }

        Log.i(TAG, "onConfigurationChanged++++++");
    }

    protected void onDestroy() {
        if (mPlayProxy != null) {
            mPlayProxy.release();
            mPlayProxy = null;
        }
        position = 0;
        super.onDestroy();
        // TODO Auto-generated method stub
        Log.v(TAG, "Player onDestroy Completed!");
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    Handler handler = new Handler() {
        public void handleMessage(Message msg) {
            if (msg.what == 1) {
                //mSeekBar.setProgress(mPlayProxy.getPosition()/mPlayProxy.duration());
                Log.e(TAG, "posion=" + String.valueOf(mPlayProxy.getPosition()));
                mSeekBar.setProgress(mPlayProxy.getPosition());



                int sec = mPlayProxy.getPosition() / 1000;
                int min = sec / 60;
                int msec = sec % 60;
                currentTimeTV.setText(min + ":" + msec);
            }
            super.handleMessage(msg);
        }

        ;
    };

    TimerTask task = new TimerTask() {

        @Override
        public void run() {
            // 需要做的事:发送消息
            Message message = new Message();
            message.what = 1;
            handler.sendMessage(message);
        }
    };

}
