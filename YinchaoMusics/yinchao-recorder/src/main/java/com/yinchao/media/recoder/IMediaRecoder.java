package com.yinchao.media.recoder;

public interface IMediaRecoder {

	/**
	 * init recoder
	 * 
	 * @param sampleRate
	 * @param channels
	 * @param samplebit
	 */
	public void initRecoder(int sampleRate, int channels, int samplebit);

	/**
	 * start rcoder
	 */
	public void startRecoder();

	/**
	 * stop recoder
	 */
	public void stopRecoder();
}
