package com.yinchao.android.media.test.utils;

/**
 * Created by Administrator on 2016/11/14.
 */

public class TimeFormatUtils {


    public static String format(int duration){
        String result = "";
        result += duration/60;
        result += ":";
        result += duration%60;
        return result;
    }


}
