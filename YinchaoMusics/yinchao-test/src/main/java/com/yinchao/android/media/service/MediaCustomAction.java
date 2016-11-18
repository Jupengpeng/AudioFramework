package com.yinchao.android.media.service;

import android.os.Bundle;

/**
 * Created by Administrator on 2016/11/18.
 */

public class MediaCustomAction {

    public static String ACTION_VOLUME = "action_volume";
    public static String VOLUME_KEY_LEFT = "volume_key_left";
    public static String VOLUME_KEY_RIGHT = "volume_key_right";

    /**
     *
     * @param bundle
     * @return
     */
    public static int castVolumeLeft(Bundle bundle) {
        int left = bundle.getInt(VOLUME_KEY_LEFT,100);
        return left;
    }

    /**
     *
     * @param bundle
     * @return
     */
    public static int castVolumeRight(Bundle bundle) {
        int right = bundle.getInt(VOLUME_KEY_RIGHT, 100);
        return right;
    }

    /**
     *
     * @param left
     * @param right
     * @return
     */
    public static Bundle buildByVolume(int left, int right){
        Bundle bundle = new Bundle();
        bundle.putInt(VOLUME_KEY_LEFT,left);
        bundle.putInt(VOLUME_KEY_RIGHT,right);
        return bundle;
    }


}
