package cn.yinchao.app.music.sample.record;

import android.media.AudioManager;
import android.media.MediaPlayer;
import android.util.Log;


import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Created by June on 2015/9/16.
 */
public class MediaInstance {
    public static MediaInstance mInstance;
    public MediaPlayer mediaPlayer;
    public boolean isPlay = false;

    public boolean asynchronization = false;
    private IMediaPlayerListener iml;


    public int seek = -1;

    MediaInstance() {

    }

    public synchronized static MediaInstance getInstance() {
        if (mInstance == null)
            mInstance = new MediaInstance();
        return mInstance;
    }

    public void startMediaPlay(final String path) {
        if (!isPlay) {
            try {
                isPlay = true;
                if (mediaPlayer == null) {
                    creatMediaPlayer(path);
                }
                mediaPlayer.prepare();
                asynchronization = false;
            } catch (IOException e) {
                isPlay = false;
//                e.printStackTrace();
            }
        }
    }

    public void seekTo(int msec) {
        if (!isPlay){
            seek = msec;
            return;
        }
        int d = mediaPlayer.getDuration();
        mediaPlayer.seekTo(Math.min(d, msec));
    }

//    public void startMediaPlayAsync(final String path) {
//
//        MyThreadPool.getInstance().doTask(new Runnable() {
//            @Override
//            public void run() {
//
//        if (!isPlay) {
//            try {
//                isPlay = true;
//                if (mediaPlayer == null) {
//                    creatMediaPlayer(path);
//                }
//                mediaPlayer.prepareAsync();
//                asynchronization = true;
//            } catch (Exception e) {
//                e.printStackTrace();
//            }
//        }
//            }
//        });
//
//    }


    public void stopMediaPlay() {
        if (mediaPlayer != null) {
            isPlay = false;
            mediaPlayer.stop();
            mediaPlayer.release();
            mediaPlayer = null;
            if (iml != null) {
                iml.onStop();
            }
        }
        isPlay = false;
    }

    public void pauseMediaPlay() {
        if (mediaPlayer != null && isPlay) {
            isPlay = false;
            mediaPlayer.pause();
            if (iml != null) {
                iml.onPause();
            }
        }
    }


    public void resumeMediaPlay() {
        if (mediaPlayer != null && !isPlay) {
            isPlay = true;
            if (seek != -1) {
                mediaPlayer.seekTo(seek);
                seek = -1;
            } else {
                mediaPlayer.start();
            }
            if (iml != null) {
                iml.onPlay();
            }
        }
    }

    public boolean isPlay() {
        if (mediaPlayer != null) {
            return isPlay;
        }
        return false;
    }

    public int getProgress() {
        if (mediaPlayer != null) {
            return (int) (mediaPlayer.getCurrentPosition() * 1.0f / mediaPlayer.getDuration() * 100);
        }
        return 0;
    }

    public void creatMediaPlayer(String file) {
        if (mediaPlayer == null) {
            mediaPlayer = new MediaPlayer();
            mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);// 设置媒体流类型

            mediaPlayer.setOnSeekCompleteListener(new MediaPlayer.OnSeekCompleteListener() {
                @Override
                public void onSeekComplete(MediaPlayer mp) {
                    if (iml!= null) iml.onPlay();
                    mediaPlayer.start();
                }
            });
            mediaPlayer.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
                @Override
                public void onPrepared(MediaPlayer mp) {
                    isPlay = true;

                    if (seek != -1) {
                        mp.seekTo(seek);
                        seek = -1;
                    } else {
                        mp.start();
                    }
                    if (iml != null) {
                        iml.onStart();
                    }
                }
            });

            mediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
                @Override
                public void onCompletion(MediaPlayer mp) {
                    isPlay = false;
//                    mp.reset();
//                    mp.release();
                    mediaPlayer = null;
                    if (iml != null) {
                        iml.onOver();
                    }
                }
            });
            mediaPlayer.setOnErrorListener(new MediaPlayer.OnErrorListener() {
                @Override
                public boolean onError(MediaPlayer mp, int what, int extra) {
                    isPlay = false;
                    mp.reset();
                    mp.release();
                    mediaPlayer = null;
                    if (iml != null) {
                        iml.onError();
                    }
                    return false;
                }
            });

            try {
                mediaPlayer.setDataSource(file);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }


    Timer timer;
    OnProgressLitsener onProgressLitsener;


    /**
     *
     */
    public void startTimerTask() {
        if (timer != null) {
            timer.cancel();
            timer = null;
        }
        timer = new Timer();

        TimerTask task = new TimerTask() {
            @Override
            public void run() {
                try {
                    if (mediaPlayer != null) {
                        int current = mediaPlayer.getCurrentPosition();
                        Log.d("timer", "createtime:" + current);
                        if (onProgressLitsener != null && current != 1) {
                            onProgressLitsener.progress(current);
                        }
                    } else {
                        Log.d("timer", "cancel");
                        timer.cancel();
                        timer = null;
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        };

        timer.schedule(task, 50, 50);

    }

    public static int getFormat(int duration){
        int time = duration/1000;
        int space = duration%1000;
        if (space > 300){
            time++;
        }
        return time;
    }


    /**
     *
     */
    public void stopTimerTask() {
        if (timer != null) {
            try{
                timer.cancel();
                timer = null;
            }catch (Exception e){

            }
        }
    }

    public void setOnProgressLitsener(OnProgressLitsener onProgressLitsener) {
        this.onProgressLitsener = onProgressLitsener;
    }

    public interface OnProgressLitsener {
        void progress(int progress);
    }


    public void destroy() {
        if (isPlay) {
            mediaPlayer.stop();
        }
        try {
            mediaPlayer.reset();
            mediaPlayer.release();
        } catch (Exception e) {
//            e.printStackTrace();
        }
        mInstance = null;
    }


    public void setIMediaPlayerListener(IMediaPlayerListener iml) {
        this.iml = iml;
    }
}
