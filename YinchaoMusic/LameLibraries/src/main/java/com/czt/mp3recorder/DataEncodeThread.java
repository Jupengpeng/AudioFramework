package com.czt.mp3recorder;

import android.media.AudioRecord;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import com.czt.mp3recorder.util.LameUtil;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.CountDownLatch;

public class DataEncodeThread extends Thread implements AudioRecord.OnRecordPositionUpdateListener {

	public static final int PROCESS_STOP = 0x01;

	private RandomAccessFile randomAccessFile;
	private StopHandler mHandler;
	private CountDownLatch mHandlerInitLatch;

	private CountDownLatch stopCount;

	private byte[] mMp3Buffer;

	private List<Task> mTasks = Collections.synchronizedList(new ArrayList<Task>());

	/**
	 * StopHandler.
	 * @see <a>https://groups.google.com/forum/?fromgroups=#!msg/android-developers/1aPZXZG6kWk/lIYDavGYn5UJ</a>
	 * @author buihong_ha
	 */
	static class StopHandler extends Handler {

		WeakReference<DataEncodeThread> encodeThread;

		public StopHandler(DataEncodeThread encodeThread) {
			this.encodeThread = new WeakReference<>(encodeThread);
		}

		@Override
		public void handleMessage(Message msg) {
			if (msg.what == PROCESS_STOP) {
				DataEncodeThread threadRef = encodeThread.get();
				//处理缓冲区中的数据
				while (threadRef.processData() > 0);
				// Cancel any event left in the queue
				removeCallbacksAndMessages(null);
				threadRef.flushAndRelease();
				getLooper().quit();
				Log.d("audio","handleMessage");
			}
			super.handleMessage(msg);
		}
	}

	/**
	 * 缓冲区数据格式.
	 * TaskByData
	 */
	private class Task{
		private short[] rawData;
		private int readSize;
		public Task(short[] rawData, int readSize){
			this.rawData = rawData.clone();
			this.readSize = readSize;
		}
		public short[] getData(){
			return rawData;
		}
		public int getReadSize(){
			return readSize;
		}
	}


	/**
	 * DataEncodeThread.
	 * @param file file
	 * @param bufferSize bufferSize
	 * @throws FileNotFoundException file not found
	 */
	public DataEncodeThread(File file, int bufferSize) throws FileNotFoundException {
		this.randomAccessFile = new RandomAccessFile(file, "rw");
		this.mHandlerInitLatch = new CountDownLatch(1);
		this.stopCount = new CountDownLatch(1);
		mMp3Buffer = new byte[(int) (7200 + (bufferSize * 2 * 1.25))];
	}

	/**
	 * 线程run方法.
	 */
	@Override
	public void run() {
		Looper.prepare();
		mHandler = new StopHandler(this);
		mHandlerInitLatch.countDown();
		Looper.loop();
	}

	/**
	 * Return the handler attach to this thread
	 * @return the handler attach to this thread
	 */
	public Handler getHandler() {
		try {
			mHandlerInitLatch.await();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		return mHandler;
	}

	public void flush(){
		try {
			stopCount.await();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onMarkerReached(AudioRecord recorder) {
		// Do nothing		
	}

	@Override
	public void onPeriodicNotification(AudioRecord recorder) {
		processData();
	}

	/**
	 * 添加数据到缓冲区.
	 * @param rawData
	 * @param readSize
     */
	public void addTask(short[] rawData, int readSize){
		mTasks.add(new Task(rawData, readSize));
	}
	/**
	 * 从缓冲区中读取并处理数据，使用lame编码MP3
	 * @return  从缓冲区中读取的数据的长度
	 * 			缓冲区中没有数据时返回0 
	 */
	private int processData() {
//		Log.d("audio","processData");
		if (mTasks.size() > 0) {
			Task task = mTasks.remove(0);
			short[] buffer = task.getData();
			int readSize = task.getReadSize();
			int encodedSize = LameUtil.encode(buffer, buffer, readSize, mMp3Buffer);


			if (encodedSize > 0){
				try {
					randomAccessFile.seek(randomAccessFile.length());
					randomAccessFile.write(mMp3Buffer, 0, encodedSize);

//					mFileOutputStream.write(mMp3Buffer, 0, encodedSize);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			return readSize;
		}
		return 0;
	}

	/**
	 * Flush all data left in lame buffer to file
	 */
	private void flushAndRelease() {
		//将MP3结尾信息写入buffer中
		final int flushResult = LameUtil.flush(mMp3Buffer);
		if (flushResult > 0) {
			try {
				randomAccessFile.seek(randomAccessFile.length());
				randomAccessFile.write(mMp3Buffer, 0, flushResult);
			} catch (IOException e) {
				e.printStackTrace();
			}finally{
				if (randomAccessFile != null) {
					try {
						randomAccessFile.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
				LameUtil.close();
			}
		}
		stopCount.countDown();
	}

}
