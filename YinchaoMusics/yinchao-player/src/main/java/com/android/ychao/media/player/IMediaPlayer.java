package com.android.ychao.media.player;

import android.view.SurfaceView;

public interface IMediaPlayer {
	
    /**
     * 异步设置播
     *
     * @param aUrl 流路.
     * @param aflag 打开参数
     * @throws Exception Exception
     */
    void setDataSourceAsync(String aUrl, int aflag) throws Exception;

    /**
     * 设置网络类型
     *
     * @param type 类型
     */
    void setActiveNetWorkType(int type);
    
    /**
     * 设置网络类型
     *
     * @param type 类型
     */
    void setDecoderType(int type);

    /**
     * 异步缓存文件路路
     *
     * @param cacheFilePath 文件路径.
     */
    void setCacheFilePath(String cacheFilePath);
    
		/**
	 	* 设置当前surfaceView
	 	* 
	 	* @param     sv	当前SurfaceView
	 	*/
        void setView(SurfaceView sv);

    /**
     *
     *
     * @return 播放操作状
     */
    int play();

    /**
     * 暂停
     */
    void pause();

    /**
     * 继续
     */
    void resume();

    /**
     * 停止
     */
    void stop();

    /**
     * seek操作
     *
     * @param aPos seek到的位置，单位毫
     */
    int setPosition(int aPos, int flag);

    /**
     * 设置播放区间
     *
     * @param aStart 起始时间
     * @param aEnd   结束时间
     */
    void setPlayRange(int aStart, int aEnd);

    /**
     * 获取播放位置
     *
     * @return 播放位置，单位毫
     */
    int getPosition();

    /**
     * 获取缓冲进度
     * @return 百分
     */
    float getBufferPercent();
    /**
     *获取缓冲大小
     * @return buffer size
     */
    int getBufferSize();

    /**
     * 获取文件大小
     * @return  文件大小
     */
    int getFileSize();

    /**
     * 歌曲时长
     *
     * @return 歌曲时长，单位毫
     */
    int duration();

    /**
     * 缓存百分
     *
     * @return 百分
     */
    int bufferedPercent();

    /**
     * 获取频谱和波
     *
     * @param aFreqarr   频谱
     * @param aWavearr   波形
     * @param aSampleNum 采样个数
     * @return 操作
     */
    int getCurFreqAndWave(short[] aFreqarr, short[] aWavearr, int aSampleNum);

    /**
     * 获取频谱
     *
     * @param aFreqarr 频谱
     * @param aFreqNum 采样个数
     * @return 操作
     */
    int getCurFreq(short[] aFreqarr, int aFreqNum);

    /**
     * 获取波形
     *
     * @param aWavearr 波形
     * @param aWaveNum 采样个数
     * @return 操作
     */
    int getCurWave(short[] aWavearr, int aWaveNum);

    /**
     * 设置音量
     *
     * @param aLVolume 左声
     * @param aRVolume 右声
     */
    void setVolume(float aLVolume, float aRVolume);

    /**
     * 获得下载数据速度
     *
     */
    int bufferedBandWidth();

    /**
     * 释放资源，调用后，播放引擎不能使
     */
    void release();

    /**
     * 设置声道平衡
     * @param balance balance
     */
    void setChannelBalance(float balance);

    /**
     * enable low delay audio effect processing
     * @param enable enable
     */
    void setAudioEffectLowDelay(boolean enable);






    /**
     * enable screen size
     * @param width
     * @param height
     */
    void setViewSize(int width, int height);
    
    /**
     * enable width size changed
     * @param width
     * @param height
     */
    void videoSizeChanged(int width, int height);

    /**
     * set ProxyServer Config parameter
     * @param ip ip
     * @param port port
     * @param authenkey authenkey
     * @param useProxy useProxy
     */
    void setProxyServerConfig(String ip, int port, String authenkey, boolean useProxy);
    
    /**
     * 清屏，一般是画黑
     *
     */
    void ClearScreen(int aClear);
}
