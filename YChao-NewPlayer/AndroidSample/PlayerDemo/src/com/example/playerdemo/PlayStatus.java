package com.example.playerdemo;

public enum PlayStatus {
    /**
     * 正在播放
     */
    STATUS_PLAYING,

    /**
     * 暂停
     */
    STATUS_PAUSED,

    /**
     * 停止，默认状态
     */
    STATUS_STOPPED,

    /**
     * 播放出错
     */
    STATUS_ERROR
}
