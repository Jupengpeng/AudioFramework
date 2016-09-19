package cn.yinchao.app.music.sample;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.ImageView;

import com.czt.mp3recorder.MP3Recorder;

import java.io.File;
import java.io.IOException;

import butterknife.Bind;
import butterknife.ButterKnife;
import butterknife.OnClick;
import cn.yinchao.app.music.sample.record.MediaInstance;
import cn.yinchao.app.music.sample.utils.PermissionUtils;
import cn.yinchao.app.music.sample.utils.ToastUtils;


/**
 * MainActivity.
 */
public class MainActivity extends AppCompatActivity {

    @Bind(R.id.iv_import_banzhou)
    ImageView ivImportBanzhou;
    @Bind(R.id.iv_play)
    ImageView ivPlay;
    @Bind(R.id.iv_record)
    ImageView ivRecord;
    @Bind(R.id.iv_restart)
    ImageView ivRestart;
    @Bind(R.id.iv_import_lrc)
    ImageView ivImportLrc;

    private MP3Recorder mRecorder;

    String fileName = "yinchaosample/temp.mp3";

    /**
     * onCreate.
     *
     * @param savedInstanceState
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);

        PermissionUtils.checkSdcardPermission(this);


        checkFile();
        mRecorder = new MP3Recorder(new File(Environment.getExternalStorageDirectory(), fileName));

        ivRecord.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                onClickRecord();
            }
        });
    }

    /**
     *
     */
//    @OnClick(R.id.iv_record)
    public void onClickRecord() {

        checkFile();

        if (ivRecord.isSelected()) {
            ivRecord.setSelected(false);
            mRecorder.stop();
        } else {
            ivRecord.setSelected(true);
            try {
                PermissionUtils.checkRecordAudioPermission(this);
                mRecorder.start();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        ToastUtils.toast(this, "onClickRecord");

    }



    @OnClick(R.id.iv_play)

    public void playMusic(){

        File file = new File(Environment.getExternalStorageDirectory(), fileName);
        MediaInstance.getInstance().startMediaPlay(file.getAbsolutePath());

    }

    @OnClick(R.id.iv_restart)
    public void clearTempFile(){
        File file = new File(Environment.getExternalStorageDirectory(), fileName);

        if (file.exists()){
            file.delete();
        }
    }

    public void checkFile() {

        File file = new File(Environment.getExternalStorageDirectory(), fileName);

        if (file.isDirectory()) {
            file.delete();
        }

        if (!file.exists()) {

            file.getParentFile().mkdirs();
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }


}
