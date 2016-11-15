package com.yinchao.media.recoder;

import android.R.string;

public interface IPcmPlayer {

	/**
	 * ���ò���·��
	 * @param recodfilePath
	 * @param accompanyFilepath
	 */
	public void setDataSourceSyn(string recodfilePath, string accompanyFilepath);
	
	
	/**
	 * ��ʼ����
	 */
	public void start();

	
	/**
	 *��ͣ ����
	 */
	public void pause();

	/**
	 * �ָ�����
	 */
	public void resume();

	
	/**
	 * ֹͣ����
	 */
	public void stop();

	
	/**
	 * ����
	 * @param mPosition
	 */
	public void seek(int mPosition);
	/**
	 * ����������С
	 * @param mVolume
	 */
	public void setVolume(int mVolume);

}
