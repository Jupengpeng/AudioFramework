package cn.yinchao.app.music.sample.record;

/**
 * Created by Zning on 2015/9/16.
 */
public interface IMediaPlayerListener {

    void onStart();

    void onStop();

    void onPlay();

    void onPause();

    void onOver();

    void onError();

}
