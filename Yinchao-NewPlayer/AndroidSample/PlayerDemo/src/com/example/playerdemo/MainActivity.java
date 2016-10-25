package com.example.playerdemo;

import java.util.Timer;
import java.util.TimerTask;

import com.android.ychao.media.player.MediaPlayerNotificationInfo;
import com.android.ychao.media.player.YCMediaPlayer;
import com.example.playerdemo.MediaPlayerProxy.OnMediaDurationUpdateListener;

import android.R.string;
import android.content.Context;
import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View.OnClickListener;
import android.view.*;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.graphics.PixelFormat;
import android.view.ViewGroup;
import android.util.DisplayMetrics;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.util.Log;

public class MainActivity extends Activity {
	private static final String TAG = "@@@Player Example";
	MediaPlayerProxy mPlayProxy = null;
	Button btnPauseResume;
	Button btnSeek;
	String defaulturl;
	String cachePath;
	EditText urlEdit;
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

		setContentView(R.layout.activity_main);
		String pluginPath = "/data/data/com.example.playerdemo/lib";
		Log.i(TAG, "onCreate" + position + "mPlayProxy :" + mPlayProxy);
		timer.schedule(task, 1000, 1000);
		if (mPlayProxy != null) {
			mPlayProxy.release();
			mPlayProxy = null;
		}
		totalTimeTV = (TextView) findViewById(R.id.textView2);
		mPlayProxy = new MediaPlayerProxy(pluginPath);

		mPlayProxy.setOnStateChangeListener(new OnStateChangeListener() {

			@Override
			public void onStarted() {
				// TODO Auto-generated method stub

			}

			@Override
			public void onSeekCompleted() {
				// TODO Auto-generated method stub

			}

			@Override
			public void onPrepared(int error, int av) {
				// TODO Auto-generated method stub
				int nDuration = mPlayProxy.duration();
				Log.e(TAG, "nDuration " + nDuration);
				int sec = nDuration/1000;
				int min =sec/60;
				int msec = sec%60;
				totalTimeTV.setText(min+":"+msec);
				mSeekBar.setMax(nDuration);
				mPlayProxy.start();
			}

			@Override
			public void onPaused() {
				// TODO Auto-generated method stub

			}

			@Override
			public void onError(int error, int httpCode,
					MediaPlayerNotificationInfo notificationInfo) {
				// TODO Auto-generated method stub

			}

			@Override
			public void onCompleted() {
				// TODO Auto-generated method stub

			}

			@Override
			public void onBufferingStarted() {
				// TODO Auto-generated method stub

			}

			@Override
			public void onBufferingDone() {
				// TODO Auto-generated method stub

			}

			@Override
			public void onBufferFinished() {
				// TODO Auto-generated method stub

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
				Log.e(TAG, "posion="+String.valueOf(mPlayProxy.getPosition()));
				mSeekBar.setProgress(mPlayProxy.getPosition());
			}
			super.handleMessage(msg);
		};
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
