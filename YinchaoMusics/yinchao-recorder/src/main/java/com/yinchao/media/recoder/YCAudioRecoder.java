package com.yinchao.media.recoder;

public class YCAudioRecoder implements IMediaRecoder {
	private final static String TAG = "YCAudioRecoder";
	static {
		try {
			System.loadLibrary("YCMediaRecoder");
		} catch (UnsatisfiedLinkError error) {
			error.printStackTrace();
		}
	}
	private int mNativeAudioManagerPara = 0;

	public YCAudioRecoder(int sampleRate, int channels, int sampleBits) {
		// TODO Auto-generated constructor stub
		initRecoder(sampleRate, channels, sampleBits);
	}

	public native void initRecord(int sampleRate, int channels, int samplebits);

	public native void startRecord(int context);

	public native void stopRecord(int context);

	@Override
	public void initRecoder(int sampleRate, int channels, int samplebit) {
		// TODO Auto-generated method stub
		initRecord(sampleRate, channels, samplebit);
	}

	@Override
	public void startRecoder() {
		// TODO Auto-generated method stub
		startRecord(mNativeAudioManagerPara);
	}

	@Override
	public void stopRecoder() {
		// TODO Auto-generated method stub
		stopRecord(mNativeAudioManagerPara);
	}
}
