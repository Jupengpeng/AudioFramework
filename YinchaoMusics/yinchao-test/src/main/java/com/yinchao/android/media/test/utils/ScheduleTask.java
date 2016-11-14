package com.yinchao.android.media.test.utils;

import android.os.Handler;
import android.os.Message;

import java.util.Timer;
import java.util.TimerTask;

/**
 * ScheduleTask.
 * Created by Administrator on 2016/11/14.
 * 执行周期 MainThread 任务.
 */

public class ScheduleTask {

    private static final int WHAT_ID = 0x022;

    private RunTask taskRun = null;
    private Timer timer = null;
    private Handler handler = null;

    /**
     * ScheduleTask.
     */
    public ScheduleTask() {

    }

    /**
     * schedule.
     *
     * @param task
     * @param delay
     * @param period
     */
    public void schedule(RunTask task, long delay, long period){
        if (handler == null){
            handler = buildHandler();
        }
        timer = new Timer();
        timer.schedule(buildScheduleTask(),delay,period);
        taskRun = task;
    }

    /**
     * cancel.
     */
    public void cancel(){
        taskRun = null;
        if (timer != null){
            timer.cancel();
            timer = null;
        }
        if (handler != null){
            handler.removeCallbacksAndMessages(null);
        }
    }

    /**
     * destroy.
     */
    public void destroy(){
        cancel();
        handler = null;
    }

    /**
     *
     * @return Handler.
     */
    private Handler buildHandler(){
        Handler handler = new Handler() {
            public void handleMessage(Message msg) {
                if (msg.what == WHAT_ID) {
                    if (taskRun != null){
                        taskRun.run();
                    }
                }
            }
        };
        return handler;
    }



    /**
     * buildScheduleTask.
     * @return TimerTask.
     */
    private TimerTask buildScheduleTask(){

        TimerTask scheduleTask = new TimerTask() {
            @Override
            public void run() {
                Message message = handler.obtainMessage(WHAT_ID);
                handler.sendMessage(message);
            }
        };

        return scheduleTask;
    }

    /**
     * Task.
     */
    public interface RunTask {
        void run();
    }


}
