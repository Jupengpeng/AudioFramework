package com.yinchao.media.recoder;

import android.R.string;

public interface IPcmPlayer {

	/**
	 * 设置播放路径
	 * @param recodfilePath
	 * @param accompanyFilepath
	 */
	public void setDataSourceSyn(string recodfilePath, string accompanyFilepath);
	
	
	/**
	 * 开始播放
	 */
	public void start();

	
	/**
	 *暂停 播放
	 */
	public void pause();

	/**
	 * 恢复播放
	 */
	public void resume();

	
	/**
	 * 停止播放
	 */
	public void stop();

	
	/**
	 * 查找
	 * @param mPosition
	 */
	public void seek(int mPosition);
	/**
	 * 设置音量大小
	 * @param mVolume
	 */
	public void setVolume(int mVolume);

}
