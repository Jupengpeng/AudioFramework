package com.yinchao.media.recoder;

import android.R.string;

public class YCPcmPlayer implements IPcmPlayer{

	private final static String TAG = "YCPcmPlayer";
	static {
		try {
			System.loadLibrary("YCPcmPlayer");
		} catch (UnsatisfiedLinkError error) {
			error.printStackTrace();
		}
	}
	
	private int mNativePcmPlayerPara = 0;

	public native void setDataSource(string recodfilePath,string accompanyFilepath);
	
	public native void start(int context);
	
	public native void seek(int mPosition,int context);
	
	public native void pause(int context);
	
	public native void resume(int context);
	
	public native void stop(int context);
	
	public native void setVolume(int mVolume,int context);
	
	
	
	@Override
	public void setDataSourceSyn(string recodfilePath, string accompanyFilepath) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void start() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void pause() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void resume() {
		// TODO Auto-generated method stub
		
	}
	
	

	@Override
	public void stop() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void setVolume(int mVolume) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void seek(int mPosition) {
		// TODO Auto-generated method stub
		
	}
	
	
}
