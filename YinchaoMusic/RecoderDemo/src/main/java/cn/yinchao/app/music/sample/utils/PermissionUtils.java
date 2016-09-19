package cn.yinchao.app.music.sample.utils;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;

/**
 * Created by hujunwei on 16/7/4.
 */
public class PermissionUtils {
    public static final int MY_PERMISSIONS_REQUEST_READ_PHONE_STATE = 1;
    public static final int MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE = 2;
    public static final int MY_PERMISSIONS_REQUEST_RECORD_AUDIO = 3;
    public static final int MY_PERMISSIONS_REQUEST_WRITE_SETTINGS = 4;
    public static final int MY_PERMISSIONS_REQUEST_CALL_PHONE = 5;
    //读取手机imei
    public static boolean checkReadPhoneStatePermission(Context activity) {
        if (ContextCompat.checkSelfPermission(activity,
                Manifest.permission.READ_PHONE_STATE)
                == PackageManager.PERMISSION_GRANTED) {
            return true;
        } else {
            ActivityCompat.requestPermissions((Activity)activity,
                    new String[]{Manifest.permission.READ_PHONE_STATE},
                    MY_PERMISSIONS_REQUEST_READ_PHONE_STATE);
            return false;
        }
    }
    public static boolean checkCallPhonePermission(Context activity) {
        if (ContextCompat.checkSelfPermission(activity,
                Manifest.permission.CALL_PHONE)
                == PackageManager.PERMISSION_GRANTED) {
            return true;
        } else {
            ActivityCompat.requestPermissions((Activity)activity,
                    new String[]{Manifest.permission.CALL_PHONE},
                    MY_PERMISSIONS_REQUEST_CALL_PHONE);
            return false;
        }
    }
    public static boolean checkSdcardPermission(Activity activity) {
        if (ContextCompat.checkSelfPermission(activity,
                Manifest.permission.READ_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(activity,
                    Manifest.permission.READ_EXTERNAL_STORAGE)) {
//                showDialog(activity,MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE);
            } else {
                ActivityCompat.requestPermissions(activity,
                        new String[]{Manifest.permission.READ_EXTERNAL_STORAGE},
                        MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE);
            }
            return false;
        } else {
            return true;
        }
    }

    public static boolean checkWriteSettingPermission(Activity activity) {
        if (ContextCompat.checkSelfPermission(activity,
                Manifest.permission.WRITE_SETTINGS)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(activity,
                    new String[]{Manifest.permission.WRITE_SETTINGS},
                    MY_PERMISSIONS_REQUEST_WRITE_SETTINGS);
            return false;
        } else {
            return true;
        }
    }

    public static boolean checkRecordAudioPermission(Activity activity) {
        if (ContextCompat.checkSelfPermission(activity,
                Manifest.permission.RECORD_AUDIO)
                != PackageManager.PERMISSION_GRANTED) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(activity,
                    Manifest.permission.READ_EXTERNAL_STORAGE)) {
//                showDialog(activity,MY_PERMISSIONS_REQUEST_RECORD_AUDIO);
            } else {
                ActivityCompat.requestPermissions(activity,
                        new String[]{Manifest.permission.RECORD_AUDIO},
                        MY_PERMISSIONS_REQUEST_RECORD_AUDIO);
            }
            return false;
        } else {
            return true;
        }
    }

//    public static void showDialog(Activity activity, int permissionType) {
//        new MaterialDialog.Builder(activity)
//                .title(activity.getResources().getString(R.string.progress_dialog))
//                .content("你的录音权限没有打开，请开启以后再继续操作!")
//                .positiveText("打开")
//                .onPositive(new MaterialDialog.SingleButtonCallback() {
//                    @Override
//                    public void onClick(@NonNull MaterialDialog dialog, @NonNull DialogAction which) {
//                        switch (permissionType) {
//                            case MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE:
//                                ActivityCompat.requestPermissions(activity,
//                                        new String[]{Manifest.permission.RECORD_AUDIO},
//                                        MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE);
//                                break;
//                            case MY_PERMISSIONS_REQUEST_RECORD_AUDIO:
//                                ActivityCompat.requestPermissions(activity,
//                                        new String[]{Manifest.permission.RECORD_AUDIO},
//                                        MY_PERMISSIONS_REQUEST_RECORD_AUDIO);
//                                break;
//                        }
//                    }
//                }).negativeText("取消")
//                .onNegative(new MaterialDialog.SingleButtonCallback() {
//                    @Override
//                    public void onClick(@NonNull MaterialDialog dialog, @NonNull DialogAction which) {
//
//                    }
//                }).show();
//    }
}
