package com.yinchao.media.recoder;

import android.app.Activity;
import android.media.AudioRecord;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;


public class MainActivity extends Activity {

	Button startRecord;
	Button stopRecord;
	Button play;
	static BufferedOutputStream bos;
	YCAudioRecoder mAudioRecoder;
	
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

//		setContentView(R.layout.main);
//
//		startRecord = (Button) findViewById(R.id.start);
//		stopRecord = (Button) findViewById(R.id.stop);
//		// play =(Button)findViewById(R.id.play);
//		mAudioRecoder = new YCAudioRecoder(44100, 1, 16);
//
//		startRecord.setOnClickListener(new OnClickListener() {
//			@Override
//			public void onClick(View v) {
//				mAudioRecoder.startRecoder();
//
//			}
//		});
//
//		stopRecord.setOnClickListener(new OnClickListener() {
//			@Override
//			public void onClick(View v) {
//				mAudioRecoder.stopRecoder();
//			}
//		});

	}

}
