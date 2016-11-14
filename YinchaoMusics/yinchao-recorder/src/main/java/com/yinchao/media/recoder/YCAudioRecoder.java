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
		initRecoder(sampleRate, channels, sampleBits);
	}

	public native void initRecord(int sampleRate, int channels, int samplebits);

	public native void startRecord(int context);

	public native void stopRecord(int context);

	@Override
	public void initRecoder(int sampleRate, int channels, int samplebit) {
		initRecord(sampleRate, channels, samplebit);
	}

	@Override
	public void startRecoder() {
		startRecord(mNativeAudioManagerPara);
	}

	@Override
	public void stopRecoder() {
		stopRecord(mNativeAudioManagerPara);
	}
}
