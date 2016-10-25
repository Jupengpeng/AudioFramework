package com.android.ychao.media.player;

/**
 * @author hao.sun
 * @version 1.0.0
 * @since 2014/7/16.
 */

public class MediaPlayerNotificationInfo {
    private String mIP;
    private String mURL;


    /**
     * construct
     *
     * @param ip  ip
     * @param url url
     */
    public MediaPlayerNotificationInfo(String ip, String url) {
        mIP = ip;
        mURL = url;
    }

    /**
     * construct
     *
     * @param obj  obj
     */
    public MediaPlayerNotificationInfo(MediaPlayerNotificationInfo obj) {
        mIP = obj.mIP;
        mURL = obj.mURL;
    }

    /**
     * 获取ip地址
     *
     * @return ip地址
     */
    public String getIP() {
        return mIP;
    }

    /**
     * 获取URL地址
     *
     * @return URL地址
     */
    public String getURL() {
        return mURL;
    }
}
