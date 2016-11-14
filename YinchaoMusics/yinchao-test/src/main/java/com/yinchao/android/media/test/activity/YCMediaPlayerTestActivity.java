package com.yinchao.android.media.test.activity;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.yinchao.android.media.R;
import com.yinchao.android.media.test.utils.ScheduleTask;

/**
 * Created by Administrator on 2016/11/14.
 */

public class YCMediaPlayerTestActivity extends AppCompatActivity {

    TextView console;
    Button button1;
    Button button2;
    Button button3;

    ScheduleTask utils = new ScheduleTask();

    private int number = 0;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mediaplayer_test);

        console = (TextView) findViewById(R.id.console);

        button1 = (Button) findViewById(R.id.start);
        button2 = (Button) findViewById(R.id.cancel);
        button3 = (Button) findViewById(R.id.cancel2);


        button1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                utils.schedule(new ScheduleTask.RunTask() {
                    @Override
                    public void run() {
                        number ++;
                        console.setText("Number:"+number);
                    }
                }, 1000, 100);
            }
        });

        button2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                utils.cancel();
            }
        });
        button3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                utils.cancel();
            }
        });
    }

}
