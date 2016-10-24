package com.sds.android.media;

import java.util.ArrayList;
import java.util.List;

import android.R.integer;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

public class MediaPlayer {
    static {
    	System.loadLibrary("mediaplayer");
    }
    
    private static final String TAG = "MediaPlayer.java";
    
    /**
     * 播放状态:正在准备
     */
    public static final int STATUS_PREPARING = 1;
    
    /**
     * 播放状态:准备完成，可以播放
     */ 
    public static final int STATUS_PREPARED = 2;
    
    /**
     * 播放状态:正在播放
     */
    public static final int STATUS_PLAYING = 3;
    
    /**
     * 播放状态:暂停
     */
    public static final int STATUS_PAUSED = 4;
    
    /**
     * 播放状态:停止
     */
    public static final int STATUS_STOPED = 5;
    
    /**
     * 录音的保存地址
     */
    public static final String PARAM_KEY_RECORD_SAVE_NAME = "record_save_name";
    
    /**
     * 背景音乐相对url的文件名
     */
    public static final String PARAM_KEY_BACKGROUND_STREAM_NAME = "background_name";
    
    /**
     * 原唱相对url的文件名
     */
    public static final String PARAM_KEY_ORIGIN_STREAM_NAME = "origin_name";
    
    /**
     * http文件缓存名
     */
    public static final String PARAM_KEY_HTTP_CACHE_NAME = "http_cache_name";
    
    /**
     * 传入参数的分隔符
     */
    public static final String  PARAM_KEY_SEPARATOR = "[,]";
    
    private static final int NOTIFY_PREPARED = 1;
    private static final int NOTIFY_STARTED = 2;
    private static final int NOTIFY_COMPLETED = 3;
    private static final int NOTIFY_PAUSED = 4;
    private static final int NOTIFY_RESUMED = 5;
    private static final int NOTIFY_EXCEPTION = 6;
    private static final int NOTIFY_RECORD_FINISHED = 7;
    
    private native void init();
    private native int createPlayer(Object self);
	private native void releasePlayer(int playerRef);
	private native void start(int playerRef);
	private native int stop(int playerRef);
	private native void pause(int playerRef);
	private native void resume(int playerRef);
	private native void seek(int playerRef, int pos);//单位ms
	private native void setAccompanimentVolume(int playerRef, int volume);
	private native void setRecorderVolume(int playerRef, int volume);
	private native int position(int playerRef);
	private native int setDataSource(int playerRef, String url, String params);
	private native int setDataSourceAsync(int playerRef, String url, String params);
	private native int duration(int playerRef);//单位ms
	private native int playStatus(int playerRef);
	private native int rhythm(int playerRef, int[] rhythm);
	private native int switch2Stream(int playerRef, String name);
	private native String getCurStreamName(int playerRef);
	private native int wave(int playerRef, int samples, short[] data);
	
	private int mInteralPlayer;
	private int[] mRhythm = new int[2]; 
	
	private List<Integer> mReadyReleaseInterPlayerList = new ArrayList<Integer>();
	
	private void checkUrl(String url) {
		if (url == null || url.equals("")) {
			throw new IllegalArgumentException("MediaPlayer url invalid!");
		}
	}
       
	/**
	 * 构造函数
	 */
    public MediaPlayer() {
		Looper looper = Looper.myLooper();
		if (looper != null) {
			mEventHandler = new EventHandler(this, null, looper);
		} else {
			looper = Looper.getMainLooper();
			mEventHandler = new EventHandler(this, null, looper);
		}
		
    	init();
    	synchronized(this) {
    		mInteralPlayer = createPlayer(this);
    	}
	}
   
    /**
     * 同步设置播放的url，文件或者网络流路径， 无OnPrepared回调
     * @param url 路径
     */
    public int setDataSource(String url) {
    	checkUrl(url);
    	return setDataSource(mInteralPlayer, url, null);
    }
    
    /**
     * seek操作
     * @param pos seek到的位置
     */
    public void setPosition(int pos) {
    	seek(mInteralPlayer, pos);
    }
    
    /**
     * 异步设置播放的url，文件或者网络流路径 完成有OnPrepared回调
     * @param url 路径
     */
    public int setDataSourceAsync(String url) {
    	checkUrl(url);
    	return setDataSourceAsync(mInteralPlayer, url, null);
	}
    
    /**
     * 同步设置播放的url，文件或者网络流路径，带有配置参数 无OnPrepared回调
     * @param url 路径
     * @param params 参数：以"k=v&k2=v2"的方式传递
     */
    public int setDataSource(String url, String params) {
    	checkUrl(url);
    	return setDataSource(mInteralPlayer, url, params);
	}
    
    /**
     * 同步设置播放的url，文件或者网络流路径，带有配置参数 有OnPrepared回调
     * @param url 路径
     * @param params 参数：以"k=v&k2=v2"的方式传递
     */
    public int setDataSourceAsync(String url, String params) {
    	checkUrl(url);
    	return setDataSourceAsync(mInteralPlayer, url, params);
	}
    
    /**
     * 同步播放歌曲
     * @param backgroundPath 背景音乐路径
     * @param originPath 原唱路径
     * @param recordSavePath 录音存储路径
     * @return 返回码
     */
    public void setDataSource(String backgroundPath, String originPath, String recordSavePath) {
    	String params = createParams(backgroundPath, originPath, recordSavePath);
    	if (0 != setDataSource("/",params)) {
    		stop();
    		setDataSource("/",params);
    	}
    }
    
    /**
     * 播放网络流
     * @param url 网络流路径
     * @param cachePath 缓存路径
     */
    public void setHttpDataSourceAsync(String url, String cachePath) {
    	String params = PARAM_KEY_HTTP_CACHE_NAME + "=" + cachePath;
    	if (0 != setDataSourceAsync(url, params)) {
    		stop();
    		setDataSourceAsync(url, params);
    	}
    }
    
    /**
     * 异步播放歌曲
     * @param backgroundPath 背景音乐路径
     * @param originPath 原唱路径
     * @param recordSavePath 录音存储路径
     * @return 返回码
     */
    public void setDataSourceAsync(String backgroundPath, String originPath, String recordSavePath) {
    	String params = createParams(backgroundPath, originPath, recordSavePath);
    	if (0 != setDataSourceAsync("/",params)) {
    		stop();
    		setDataSourceAsync("/",params);
    	}
	}
    
    private String createParams(String backgroundPath, String originPath, String recordSavePath) {
    	checkUrl(backgroundPath);
    	StringBuilder params = new StringBuilder(MediaPlayer.PARAM_KEY_BACKGROUND_STREAM_NAME + "=" + backgroundPath);
    	if (originPath != null && !originPath.equals("")) {
    		params.append(MediaPlayer.PARAM_KEY_SEPARATOR + MediaPlayer.PARAM_KEY_ORIGIN_STREAM_NAME + "=" + originPath);
		}
    	
    	if (recordSavePath != null && !recordSavePath.equals("")) {
    		params.append(MediaPlayer.PARAM_KEY_SEPARATOR + MediaPlayer.PARAM_KEY_RECORD_SAVE_NAME + "=" + recordSavePath);
		}
    	return params.toString();
	}
    
    /**
     * 启动播放
     */
    public void start() {
    	start(mInteralPlayer);
	}
    
    /**
     * 停止播放
     */
    public void stop() {
    	synchronized (this) {
    		if (0 != stop(mInteralPlayer)) {
    			Log.i(TAG, "Ready Release " + mInteralPlayer);
    			mReadyReleaseInterPlayerList.add(mInteralPlayer);
    			mInteralPlayer = createPlayer(this);
			}
    	}
	}
    
    /**
     * 暂停播放
     */
    public void pause() {
		pause(mInteralPlayer);
	}
    
    /**
     * 继续播放
     */
    public void resume() {
    	resume(mInteralPlayer);
    }
    
    /**
     * 获取播放
     */
    public int position() {
    	synchronized (this) {
    		return position(mInteralPlayer);
    	}
    }
    	
    
    /**
     * 设置伴奏音量大小
     * @param volume 音量值
     */
    public void setAccompanimentVolume(int volume) {
    	setAccompanimentVolume(mInteralPlayer, volume);
    }
    
    /**
     * 设置录音音量大小
     * @param volume 音量值
     */
    public void setRecorderVolume(int volume) {
    	setRecorderVolume(mInteralPlayer, volume);
    }
    
    /**
     * 获取正在播放音频PCM数据
     */
    public int wave(int samples, short[] data) {
    	return wave(mInteralPlayer, samples, data);
	}
    
    /**
     * 获取时长
     * @return 媒体时长
     */
    public int duration() {
    	return duration(mInteralPlayer);
	}
    
    /**
     * 获取当前播放状态
     * @return 播放状态
     */
    public int status() {
    	return playStatus(mInteralPlayer);
    }
    
    /**
     * 切换指定名字流
     * @param streamName
     * @return  0表示操作成功
     */
    public int switch2Stream(String streamName) {
    	return switch2Stream(mInteralPlayer, streamName);
    }
    
    /**
     * 获取当前播放流的名称
     * @return 当前流名称
     */
    public String getCurStreamName() {
    	return getCurStreamName(mInteralPlayer);
    }
    
    /**
     * 释放，调用后，不能再对改进行操作
     */
    public void release() {
    	synchronized (this) {
    		releasePlayer(mInteralPlayer);
    		mInteralPlayer = 0;
		}
	}
    
    /**
     * 获取旋律，
     * @return 返回操作码
     */
    public int[] rhythm() {
    	synchronized (this) {
			if (mInteralPlayer != 0 && rhythm(mInteralPlayer, mRhythm) == 0) {
				return mRhythm;
			}
		}
    	
    	return null;
	}
    
    private EventHandler eventHandler() {
		return mEventHandler;
	}
    
    private int getInteralPlayerRef() {
    	return mInteralPlayer;
    }
    
    private List<Integer> getReadyReleasePlayerRefList() {
    	return mReadyReleaseInterPlayerList;
    }
    
    private EventHandler mEventHandler;
    private static class EventHandler extends Handler {
    	private MediaPlayerObserver mObserver;
    	private MediaPlayer mHolder;
    	
		public EventHandler(MediaPlayer holder, MediaPlayerObserver observer, Looper looper) {
			super(looper);
			mHolder = holder;
			mObserver = observer;
		}
		
		public void setObserver(MediaPlayerObserver observer) {
			mObserver = observer;
		}
		
		@Override
		public void handleMessage(Message msg) {
			if (mHolder.getInteralPlayerRef() != msg.arg1) {
				List<Integer> list = mHolder.getReadyReleasePlayerRefList();
				int interalPlayerRef = msg.arg1;
				Log.i(TAG, "Need Release " + interalPlayerRef + " msgId:" + msg.what);
				for (int i = list.size() - 1; i >= 0; i--) {
					if (interalPlayerRef == list.get(i).intValue()) {
						synchronized (mHolder) {
							mHolder.releasePlayer(msg.arg1);
						}
						
						if (mObserver != null && msg.what == NOTIFY_RECORD_FINISHED) {
							mObserver.onRecordFinished(msg.arg2);//如果是录音，则需要回调，不然丢信息
						}
						list.remove(i);
						break;
					}
				}
			} else {
				if (mObserver != null) {
					switch (msg.what) {
					case NOTIFY_RECORD_FINISHED:
						mObserver.onRecordFinished(msg.arg2);
						break;
						
					case NOTIFY_PREPARED:
						mObserver.onPrepeared();
						break;
						
					case NOTIFY_COMPLETED:
						mObserver.onCompleted();
						break;
					
					case NOTIFY_PAUSED:
						mObserver.onPaused();
						break;
					
					case NOTIFY_RESUMED:
					case NOTIFY_STARTED:
						mObserver.onStarted();
						break;

					default:
						break;
					}
				}
			}
		}
    };
    
    private static void postEventFromNative(Object selfRef, int msg, int arg1, int arg2, Object obj) {
    	Log.d(TAG, "postEventFromNative:" + msg + "," + arg1 + "," + arg2);
    	
    	android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_DISPLAY);
    	MediaPlayer player = (MediaPlayer) selfRef;
		if (player != null && player.eventHandler() != null) {
			player.eventHandler().sendMessage(player.eventHandler().obtainMessage(msg, arg1, arg2, obj));
		}
	}
    
    /**
     * 设置MediaPlayer的监听接口
     * @param observer 接口实例
     */
    public void setObserver(MediaPlayerObserver observer) {
    	mEventHandler.setObserver(observer);
    }
}
