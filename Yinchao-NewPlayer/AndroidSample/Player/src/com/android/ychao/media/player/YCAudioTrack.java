package com.android.ychao.media.player;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.*;
import android.os.Process;
import android.util.Log;

import java.util.Random;


public class YCAudioTrack {
	private final static String LOG_TAG = "YCAudioTrack";

	private static final int ERR_ACCESS_DENIED = -21;
	private static final int ERR_NONE = 0;
	private static final int MIN_HARDWARE_VOLUME = -990;
	private static final float HARDWARE_COFF = 990.0f;

	private static final int BUFFER_SIZE = 1024 * 40;	

	private static int mAudioSessionId;
	private AudioTrack mAudioTrack;
	
	private int mSampleRate;
	private int mChannels;
	private int mMinBufferSize;
	
	private boolean mSetPriority = false;

	private static final int SAMPLERATE_48K = 48000;
	private static final int DEFAULT_CH = 2;

	/**
	 * Return the audio session id
	 * 
	 * @return The audio session id
	 */
	public static int audioSessionId() {
		return mAudioSessionId;
	}

	/**
	 * ���캯��
	 */
	public YCAudioTrack() {
		
	}
	
	public static int maxOutputSamplerate() {
        int maxSamplerate ;
        int audioSessionId = Math.abs(new Random().nextInt());
        if (audioSessionId == 0) {
            audioSessionId = Integer.MAX_VALUE;
        }
        try {
            	AudioTrack pAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, SAMPLERATE_48K, DEFAULT_CH,
                    AudioFormat.ENCODING_PCM_16BIT, BUFFER_SIZE, AudioTrack.MODE_STREAM);
            	maxSamplerate = SAMPLERATE_48K;
            	pAudioTrack.release();
        } catch (Exception e) {
            maxSamplerate = AudioTrack.getNativeOutputSampleRate(AudioManager.STREAM_MUSIC);
        }
        return maxSamplerate;
    }

	private int createAudioTrack(int aSampleRate, int aChannels) {
		int nChannelConfigure = ((aChannels == 1) ? AudioFormat.CHANNEL_OUT_MONO
				: AudioFormat.CHANNEL_OUT_STEREO);
				
		mMinBufferSize = AudioTrack.getMinBufferSize(aSampleRate, nChannelConfigure, AudioFormat.ENCODING_PCM_16BIT);                
		
		if ( mMinBufferSize == AudioTrack.ERROR_BAD_VALUE || mMinBufferSize == AudioTrack.ERROR )   
		     return ERR_ACCESS_DENIED;
		     
		int nMinBuf = mMinBufferSize*2;
		
		if(nMinBuf < 2048)
			nMinBuf = 2048;

		try {
			AudioTrack prevAudioTrack = mAudioTrack;

			if (Build.VERSION.SDK_INT < Build.VERSION_CODES.GINGERBREAD) {
				mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
						aSampleRate, nChannelConfigure,
						AudioFormat.ENCODING_PCM_16BIT, nMinBuf,
						AudioTrack.MODE_STREAM);
				Log.d(LOG_TAG, "createAudioTrack-1: " + aSampleRate + " - "
						+ aChannels);
			} else if (mAudioSessionId != 0) {
				mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
						aSampleRate, nChannelConfigure,
						AudioFormat.ENCODING_PCM_16BIT, nMinBuf,
						AudioTrack.MODE_STREAM, mAudioSessionId);
				Log.d(LOG_TAG, "createAudioTrack-2: " + aSampleRate + " - "
						+ aChannels);
			} else {
				do {
					mAudioSessionId = Math.abs(new Random().nextInt());
					if (mAudioSessionId == 0) {
						mAudioSessionId = Integer.MAX_VALUE;
					}

					mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
							aSampleRate, nChannelConfigure,
							AudioFormat.ENCODING_PCM_16BIT, nMinBuf,
							AudioTrack.MODE_STREAM, mAudioSessionId);
					Log.d(LOG_TAG, "createAudioTrack-3: " + aSampleRate + " - "
							+ aChannels);
				} while (mAudioSessionId != mAudioTrack.getAudioSessionId());
			}
			if (prevAudioTrack != null) {
				prevAudioTrack.release();
			}
		} catch (Exception e) {
			e.printStackTrace();
			return ERR_ACCESS_DENIED;
		}
		
		if(mAudioTrack.getState() == AudioTrack.STATE_UNINITIALIZED)
			return ERR_ACCESS_DENIED;
		
		mSetPriority =  false;
		return ERR_NONE;
	}
	
	private void writeData (byte[] audioData, int nSize)
	{
		if(mSetPriority == false) {
			Process.setThreadPriority(Process.THREAD_PRIORITY_AUDIO);
			mSetPriority = true;
		}
		if (mAudioTrack != null && nSize > 0)
		{
			if (mAudioTrack.getPlayState() != AudioTrack.PLAYSTATE_PLAYING)
			{
				mAudioTrack.play();	
			}
			
			try { 
				mAudioTrack.write(audioData, 0, nSize);
			} catch (Exception e) {  
        e.printStackTrace();  
      }  
		}
	}	

	private int audioOpen(int aSampleRate, int aChannels) {
		mSampleRate = aSampleRate;
		mChannels = aChannels;

		int err = createAudioTrack(aSampleRate, aChannels);
		if (err != ERR_NONE) {
			return err;
		}
		
		return ERR_NONE;
	}

	private void audioClose() {			
		if(mAudioTrack == null)
			return;
			
		try {	
			audioStop();
			mAudioTrack.release();
	    }catch(Exception e)
	    {
	    	e.printStackTrace();
	    }

			mAudioTrack = null;
	}

	private void audioStop() {
		if(mAudioTrack == null)
			return;
				
		try {
			mAudioTrack.stop();
			mSetPriority = false;
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private void audioSetVolume(int aLVolume, int aRVolume) {
		if (mAudioTrack != null) {
			float fLVolume;
			float fRVolume;

			if (aLVolume == 0) {
				fLVolume = 1.0f;
			} else if (aLVolume == MIN_HARDWARE_VOLUME) {
				fLVolume = 0.0f;
			} else {
				fLVolume = (aLVolume - MIN_HARDWARE_VOLUME) / HARDWARE_COFF;
			}

			if (aRVolume == 0) {
				fRVolume = 1.0f;
			} else if (aRVolume == MIN_HARDWARE_VOLUME) {
				fRVolume = 0.0f;
			} else {
				fRVolume = (aRVolume - MIN_HARDWARE_VOLUME) / HARDWARE_COFF;
			}

			try {
				mAudioTrack.setStereoVolume(fLVolume, fRVolume);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
}
