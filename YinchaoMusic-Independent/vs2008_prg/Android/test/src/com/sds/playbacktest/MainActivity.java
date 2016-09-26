package com.sds.playbacktest;

import com.sds.android.media.MediaPlayer;
import com.sds.android.media.MediaPlayerObserver;

import android.media.AudioManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.R.integer;
import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.View;
import android.view.View.MeasureSpec;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class MainActivity extends Activity implements MediaPlayerObserver {

	MediaPlayer mPlayBack = new MediaPlayer();
	
	Handler mHandler = new Handler() {
		public void handleMessage(android.os.Message msg) {
//			((SeekBar)findViewById(R.id.id_progress)).setProgress(mPlayBack.position() * 1000 / 240000);
			mPlayBack.rhythm();
			mHandler.sendMessageDelayed(Message.obtain(mHandler, 0), 200);
		};
	};
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		findViewById(R.id.id_play).setOnClickListener(mClickListener);
		findViewById(R.id.id_pause).setOnClickListener(mClickListener);
		findViewById(R.id.id_resume).setOnClickListener(mClickListener);
		findViewById(R.id.id_stop).setOnClickListener(mClickListener);
		findViewById(R.id.id_background).setOnClickListener(mClickListener);
		findViewById(R.id.id_origin).setOnClickListener(mClickListener);
		findViewById(R.id.id_mix).setOnClickListener(mClickListener);
		findViewById(R.id.id_mix2).setOnClickListener(mClickListener);
	
		((SeekBar)findViewById(R.id.id_player_volume)).setOnSeekBarChangeListener(mSeekBarChangeListener);
		((SeekBar)findViewById(R.id.id_recorder_volume)).setOnSeekBarChangeListener(mSeekBarChangeListener);
		
		mPlayBack.setObserver(this);
	}	
	
	OnSeekBarChangeListener mSeekBarChangeListener = new OnSeekBarChangeListener() {
		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
			
//			AudioManager am = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
//			long max = am.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
//			max *= seekBar.getProgress();		
//			am.setStreamVolume(AudioManager.STREAM_MUSIC, (int)(max/seekBar.getMax()), 0);
			mPlayBack.setPosition(20000);
		}
		
		@Override
		public void onStartTrackingTouch(SeekBar seekBar) {
			
		}
		
		@Override
		public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
			if (seekBar.getId() == R.id.id_player_volume) {
				
				//mPlayBack.setAccompanimentVolume(progress);
			} else if (seekBar.getId() == R.id.id_recorder_volume) {
				mPlayBack.setRecorderVolume(progress);
			}
		}
		
	};
	
	
	private View.OnClickListener mClickListener = new View.OnClickListener() {
		
		@Override
		public void onClick(View v) {
			switch (v.getId()) {
			case R.id.id_play:
				mPlayBack.setDataSourceAsync("/", MediaPlayer.PARAM_KEY_BACKGROUND_STREAM_NAME + "=" + "/sdcard/background.aac"
			);
				
				mHandler.sendMessageDelayed(new Message(), 50);
				break;
				
			case R.id.id_pause:
				mPlayBack.pause();
				break;
				
			case R.id.id_resume:
				mPlayBack.resume();
				break;
				
			case R.id.id_stop:
				mPlayBack.stop();
				break;
				
			case R.id.id_mix:
				mPlayBack.setDataSource("/sdcard/test.wav");
				mPlayBack.start();
				break;
				
			case R.id.id_mix2:
				mPlayBack.setDataSource("/sdcard/", MediaPlayer.PARAM_KEY_BACKGROUND_STREAM_NAME + "=" + "test2.aac");
				mPlayBack.start();
				break;
				
			case R.id.id_origin:
				mPlayBack.switch2Stream("origin.aac");
				break;
				
			case R.id.id_background:
				mPlayBack.switch2Stream("background.aac");
				break;
				
			default:
				break;
			}
		}
	};
	
	@Override
	public void onBackPressed() {
		super.onBackPressed();
		mHandler.removeMessages(0);
		mPlayBack.stop();
		mPlayBack.release();
		mPlayBack = null;
	};
	
	@Override
	public void onPrepeared() {
		mPlayBack.start();
	}
	
	@Override
	public void onStarted() {
		Log.d("sss", "onStarted");
	}
	
	@Override
	public void onPaused() {
		
	}
	@Override
	public void onCompleted() {
		
	}

	@Override
	public void onError(int err) {
		
	}

	@Override
	public void onRecordFinished(int duration) {
		Log.e("sss", "duration:" + duration);
	}

}
