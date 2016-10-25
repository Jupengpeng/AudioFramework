package com.example.playerdemo;

import android.util.Log;

import com.android.ychao.media.player.IMediaPlayer;
import com.android.ychao.media.player.MediaPlayerNotificationInfo;
import com.android.ychao.media.player.Normalizer;
import com.android.ychao.media.player.YCMediaPlayer;

public final class MediaPlayerProxy {

	public static final int HUNDRED = 100;
	private final int  K_FREQUENCY_NUMBER = 512;
	private short[] mFreqBuffer = new short[K_FREQUENCY_NUMBER];
	private PlayStatus mPlayStatus = PlayStatus.STATUS_STOPPED;
	private volatile YCMediaPlayer mTTMediaPlayer;
	private IMediaPlayer mCurMediaPlayer;
	private static final String TAG = "MediaPlayerDemo";

    public MediaPlayerProxy(String pluginPath) {
    	mCurMediaPlayer = buildTTMediaPlayer(pluginPath);
    }
    
    public void play(String sourcePath) throws Exception {
    	if(mCurMediaPlayer != null)
    		mCurMediaPlayer.setDataSourceAsync(sourcePath, YCMediaPlayer.OPEN_DEFAULT);
    }
    
    public void play(String sourcePath, String cacheFilePath) throws Exception {
    	if(mCurMediaPlayer != null) {
    		mCurMediaPlayer.setCacheFilePath(cacheFilePath);
    		mCurMediaPlayer.setDataSourceAsync(sourcePath, YCMediaPlayer.OPEN_DEFAULT);
    	}
    }
    
    public void stop() {
        //if(mCurMediaPlayer != null)
        	mCurMediaPlayer.stop();
        mPlayStatus = PlayStatus.STATUS_STOPPED;
    }
    
    public void palySong(String sourcePath) throws Exception{
    	stop();
        play(sourcePath);
        mPlayStatus = PlayStatus.STATUS_STOPPED;
    }
    
    public void palySong(String sourcePath, String cacheFilePath) throws Exception{
    	stop();
        play(sourcePath, cacheFilePath);
        mPlayStatus = PlayStatus.STATUS_STOPPED;
    }
    
    public void pause() {
    	if(mCurMediaPlayer != null)
    		mCurMediaPlayer.pause();
        mPlayStatus = PlayStatus.STATUS_PAUSED;
    }
    
    
    public int duration() {
    	return mCurMediaPlayer.duration();
    }
    
    
    public void start() {
        mCurMediaPlayer.play();
        mPlayStatus = PlayStatus.STATUS_PLAYING;
    }
    
    public void resume() {
        mCurMediaPlayer.resume();
        mPlayStatus = PlayStatus.STATUS_PLAYING;         
    }
    
    public PlayStatus getPlayStatus() {
        return mPlayStatus;
    }
    
    
    public int getPosition() {
        if (mCurMediaPlayer != null) {  //锟斤拷锟竭筹拷锟斤拷要锟劫界处锟斤拷
            return mCurMediaPlayer.getPosition();
        }
        return 0;
    }
    
    public float getBufferPercent() {
        if (mPlayStatus == PlayStatus.STATUS_PLAYING || mPlayStatus == PlayStatus.STATUS_PAUSED) {
            return mCurMediaPlayer.getBufferPercent();
        } else {
            return 0.0f;
        }
    }
     
	public void setPosition(int position, int flag) {
		mCurMediaPlayer.setPosition(position, flag);
	}
	
    public void setPlayRange(int aStart, int aEnd) {
        mCurMediaPlayer.setPlayRange(aStart, aEnd);
    }

    
    public void setOnStateChangeListener(OnStateChangeListener listener){
    	mStateChangeListener =listener;
    	mTTMediaPlayer.setOnMediaPlayerNotifyEventListener(mMediaPlayerNotifyEventListener);
    }
    
    private OnStateChangeListener mStateChangeListener;
//	private OnStateChangeListener mStateChangeListener = new OnStateChangeListener() {
//
//		@Override
//		public void onPrepared(int error, int av) {
//			// mediaItem ;
//
//			int lastPlayPosition = 0;// getLastPlayPosition();
//			if (lastPlayPosition != 0) {
//				setPosition(lastPlayPosition, YCMediaPlayer.SEEK_FAST);
//			}
//			
//			mCurMediaPlayer.setActiveNetWorkType(2);
//			
//			//mCurMediaPlayer.setAudioEffectLowDelay(true);
//			
//			int nDuration = mCurMediaPlayer.duration();
//			Log.e(TAG, "nDuration " + nDuration);
//			
//			start();
//		}
//
//		@Override
//		public void onStarted() {
//		}
//
//		@Override
//		public void onPaused() {
//			// saveLastPlayPosition
//		}
//		
//		@Override
//		public void onSeekCompleted() {
//			// select next song
//		}
//
//
//		@Override
//		public void onCompleted() {
//			// select next song
//		}
//
//
//
//		@Override
//		public void onBufferingStarted() {
//			Log.e(TAG, "onBufferingStarted++++++");
//		}
//
//		@Override
//		public void onBufferingDone() {
//			Log.e(TAG, "onBufferingDone-----");
//		}
//
//		@Override
//		public void onBufferFinished() {
//			// save cached file
//		}
//
//		@Override
//		public void onError(
//				int error,
//				int httpCode,
//				MediaPlayerNotificationInfo notificationInfo) {
//			// TODO Auto-generated method stub
//			
//		}
//		
//
//	};

	public interface OnMediaDurationUpdateListener {
		void onMediaDurationUpdated(int duration);
	}

	
	public void setOnMediaDurationUpdateListen(OnMediaDurationUpdateListener listen){
		mMediaDurationUpdateListener =listen;
	}
	private OnMediaDurationUpdateListener mMediaDurationUpdateListener =null;

	private YCMediaPlayer buildTTMediaPlayer(String pluginPath) {
		byte[] headerBytes = null;
		YCMediaPlayer mediaPlayer = new YCMediaPlayer(headerBytes, pluginPath);
		mTTMediaPlayer = mediaPlayer;
		//mediaPlayer.setOnMediaPlayerNotifyEventListener(mMediaPlayerNotifyEventListener);
		return mediaPlayer;
	}

	private void releaseTTMediaPlayer() {
		if (mTTMediaPlayer != null) {
			mTTMediaPlayer.release();
			mTTMediaPlayer = null;
		}
	}
	
    public void release() {
        releaseTTMediaPlayer();
        mCurMediaPlayer = null;
    }
    
    public void setAudioEffectLowDelay(boolean enable) {
    	if (mTTMediaPlayer != null) {
			mTTMediaPlayer.setAudioEffectLowDelay(enable);
		}
    }

	private YCMediaPlayer.OnMediaPlayerNotifyEventListener mMediaPlayerNotifyEventListener = new YCMediaPlayer.OnMediaPlayerNotifyEventListener() {
		@Override
		public void onMediaPlayerNotify(int aMsgId, int aArg1, int aArg2,
				Object aObj) {
			Log.d(TAG, "MsgId:" + aMsgId);
			switch (aMsgId) {
			case YCMediaPlayer.MEDIA_PREPARE:
				if (mStateChangeListener != null) {
					mStateChangeListener.onPrepared(aArg1, aArg2);
				}

				break;

			case YCMediaPlayer.MEDIA_PAUSE:
				mPlayStatus = PlayStatus.STATUS_PAUSED;
				if (mStateChangeListener != null) {
					mStateChangeListener.onPaused();
				}
				break;

			case YCMediaPlayer.MEDIA_PLAY:
				mPlayStatus = PlayStatus.STATUS_PLAYING;
				if (mStateChangeListener != null) {
					mStateChangeListener.onStarted();
				}
				break;

			case YCMediaPlayer.MEDIA_COMPLETE:
				mPlayStatus = PlayStatus.STATUS_STOPPED;
	
				if (mStateChangeListener != null) {
					mStateChangeListener.onCompleted();
				}

				break;
				
			case YCMediaPlayer.MEDIA_SEEK_COMPLETED:
				if (mStateChangeListener != null) {
					mStateChangeListener.onSeekCompleted();
				}
				
				break;

			case YCMediaPlayer.MEDIA_EXCEPTION:
				//mPlayStatus = PlayStatus.STATUS_STOPPED;

				if (mStateChangeListener != null) {
					mStateChangeListener.onError(aArg1, aArg2, (MediaPlayerNotificationInfo) aObj);
				}

				break;

			case YCMediaPlayer.MEDIA_BUFFERING_START:
				if (mStateChangeListener != null) {
					mStateChangeListener.onBufferingStarted();
				}

				break;

			case YCMediaPlayer.MEDIA_BUFFERING_DONE:
				if (mStateChangeListener != null) {
					mStateChangeListener.onBufferingDone();
				}
				break;

			case YCMediaPlayer.MEDIA_UPDATE_DURATION:
				if (mMediaDurationUpdateListener != null) {
					mMediaDurationUpdateListener.onMediaDurationUpdated(mCurMediaPlayer.duration());
				}
				break;

			case YCMediaPlayer.MEDIA_CACHE_COMPLETED:
				if (mStateChangeListener != null) {
					mStateChangeListener.onBufferFinished();
				}

				break;				

			case YCMediaPlayer.MEDIA_CLOSE:
				Log.e(TAG, "MEDIA_CLOSE: proxysize " + aArg2);
				break;
				
			case YCMediaPlayer.MEDIA_START_RECEIVE_DATA:

				break;
			case YCMediaPlayer.MEDIA_PREFETCH_COMPLETED:
	
				break;

			case YCMediaPlayer.MEDIA_DNS_DONE:

				break;
			case YCMediaPlayer.MEDIA_CONNECT_DONE:
				
				break;
			case YCMediaPlayer.MEDIA_HTTP_HEADER_RECEIVED:
				
				break;

			default:
				break;
			}
		}
	};
	
    public boolean getFreq(int[] freq, int length) {
        if (freq.length < length) {
            return false;
        }

        if (0 == mCurMediaPlayer.getCurFreq(mFreqBuffer, K_FREQUENCY_NUMBER)) {
        	int nNormalize = Normalizer.normalizeFreqBin(freq, length, mFreqBuffer, K_FREQUENCY_NUMBER);
                return 0 == nNormalize;
        }

        return false;
    }
    
    
    public boolean getWave(short[] wave, int length) {
        if (wave.length < length) {
            return false;
        }

        return 0 == mCurMediaPlayer.getCurWave(wave, length);
    }
    
    
    private void setVolume(float leftVolume, float rightVolume) {
        mCurMediaPlayer.setVolume(leftVolume, rightVolume);
    }
    

}
