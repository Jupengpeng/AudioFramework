package cn.yinchao.app.music.sample.utils;

import android.content.Context;
import android.widget.Toast;

public class ToastUtils {

    /**
     *
     * @param context
     * @param text
     * @param type
     */
    public static void toast(Context context, String text, int type) {
        if(context==null)return;

        Toast toast =  Toast.makeText(context,text,Toast.LENGTH_SHORT);
        toast.setDuration(type==0? Toast.LENGTH_SHORT: Toast.LENGTH_LONG);
        toast.show();
    }

    /**
     *
     * @param context
     * @param msg
     */
    public static void toast(Context context, String msg) {
        toast(context,msg,0);
    }

    /**
     *
     * @param context
     * @param msg
     */
    public static void toastLong(Context context, String msg) {
        toast(context,msg,1);
    }
}