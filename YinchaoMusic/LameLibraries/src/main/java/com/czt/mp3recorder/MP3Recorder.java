package com.czt.mp3recorder;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Message;
import android.util.Log;

import com.czt.mp3recorder.util.LameUtil;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class MP3Recorder {
    //=======================AudioRecord Default Settings=======================
    private static final int DEFAULT_AUDIO_SOURCE = MediaRecorder.AudioSource.DEFAULT;
    /**
     * 以下三项为默认配置参数。Google Android文档明确表明只有以下3个参数是可以在所有设备上保证支持的。
     */
    private static final int DEFAULT_SAMPLING_RATE = 44100;//模拟器仅支持从麦克风输入8kHz采样率


    private static final int DEFAULT_CHANNEL_CONFIG = AudioFormat.CHANNEL_IN_MONO;
    /**
     * 下面是对此的封装
     * private static final int DEFAULT_AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;
     */
    private static final PCMFormat DEFAULT_AUDIO_FORMAT = PCMFormat.PCM_16BIT;

    //======================Lame Default Settings=====================
    private static final int DEFAULT_LAME_MP3_QUALITY = 7;
    /**
     * 与DEFAULT_CHANNEL_CONFIG相关，因为是mono单声，所以是1
     */
    private static final int DEFAULT_LAME_IN_CHANNEL = 1;
    /**
     * Encoded bit rate. MP3 file will be encoded with bit rate 32kbps
     */
    private static final int DEFAULT_LAME_MP3_BIT_RATE = 32;

    //==================================================================

    /**
     * 自定义 每160帧作为一个周期，通知一下需要进行编码
     */
    private static final int FRAME_COUNT = 440;


    private AudioRecord mAudioRecord = null;
    private DataEncodeThread dataEncodeThread;
    private File localRecordCacheFile;

    private boolean isRecording = false;
    private int bufferSize;
    private int sampleRate = 100;
    private int readFrame = 441;

    private short[] cachePCMBuffer;

    public ArrayList<Short> buf = new ArrayList<>();
    private ArrayList<Short> dataBuffer = new ArrayList<>();

    /**
     * Default constructor. Setup recorder with default sampling rate 1 channel,
     * 16 bits pcm
     *
     * @param recordFile target file
     */
    public MP3Recorder(File recordFile) {
        localRecordCacheFile = recordFile;
//        initAudioRecorder();
    }


    /**
     * Initialize audio recorder
     */
    private void initAudioRecorder() {


//        MediaRecorder.AudioSource.MIC


        bufferSize = AudioRecord.getMinBufferSize(DEFAULT_SAMPLING_RATE,
                DEFAULT_CHANNEL_CONFIG, DEFAULT_AUDIO_FORMAT.getAudioFormat());

        Log.d("audio", "getMinBufferSize:" + bufferSize);

//        int bytesPerFrame = DEFAULT_AUDIO_FORMAT.getBytesPerFrame();
        /* Get number of samples. Calculate the buffer size
         * (round up to the factor of given frame size)
		 * 使能被整除，方便下面的周期性通知
		 * */
//        int frameSize = bufferSize / bytesPerFrame;
//        if (frameSize % FRAME_COUNT != 0) {
//            frameSize += (FRAME_COUNT - frameSize % FRAME_COUNT);
//            bufferSize = frameSize * bytesPerFrame;
//        }

//        if (bufferSize < readFrame*5){
//            bufferSize = readFrame*5;
//            Log.d("audio","bufferSize:"+readFrame);
//        }


		/* Setup audio recorder */
        mAudioRecord = new AudioRecord(DEFAULT_AUDIO_SOURCE,
                DEFAULT_SAMPLING_RATE, DEFAULT_CHANNEL_CONFIG, DEFAULT_AUDIO_FORMAT.getAudioFormat(),
                bufferSize);

        cachePCMBuffer = new short[bufferSize];
        /*
         * Initialize lame buffer
		 * mp3 sampling rate is the same as the recorded pcm sampling rate
		 * The bit rate is 32kbps
    *
            */
        LameUtil.init(DEFAULT_SAMPLING_RATE, DEFAULT_LAME_IN_CHANNEL, DEFAULT_SAMPLING_RATE, DEFAULT_LAME_MP3_BIT_RATE, DEFAULT_LAME_MP3_QUALITY);
        // Create and run thread used to encode data
        // The thread will
        try {
            dataEncodeThread = new DataEncodeThread(localRecordCacheFile, bufferSize);
            dataEncodeThread.start();
            mAudioRecord.setRecordPositionUpdateListener(dataEncodeThread, dataEncodeThread.getHandler());
        } catch (Exception e) {
            e.printStackTrace();
        }
        mAudioRecord.setPositionNotificationPeriod(readFrame);
    }


    /**
     * Start recording. Create an encoding thread. Start record from this
     * thread.
     *
     * @throws IOException initAudioRecorder throws
     */

    int index = 0;

    public void start() throws IOException {

        if (isRecording) {
            return;
        }

        initAudioRecorder();
        mAudioRecord.startRecording();

        new Thread() {
            @Override
            public void run() {
                //设置线程权限
                android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);


                isRecording = true;
                while (isRecording) {

                    Log.i("mp3","isRecording");

                    int readSize = mAudioRecord.read(cachePCMBuffer, 0, readFrame);
                    if (readSize > 0) {
                        dataEncodeThread.addTask(cachePCMBuffer, readSize);
                    }
                    if (index < 4) {
                        index++;
                        continue;
                    }
                    index = 0;
                    synchronized (dataBuffer) {
                        dataBuffer.add(calculateRealVolume(cachePCMBuffer, readSize));
                    }
                    synchronized (dataBuffer) {
                        if (dataBuffer.size() == 0) {
                            continue;
                        }
                        if (isRecording) {
                            buf = (ArrayList<Short>) dataBuffer.clone();// 保存
                        }
                    }
                    if (mOnWaveChangeListener != null) {
                        mOnWaveChangeListener.onChange(buf);
                    }
                }
                flush();
            }
        }.start();
    }

    public void prepare() throws IOException {
//        if (isRecording) return;
//        initAudioRecorder();
    }

    public void reStart() throws IOException {
        start();
    }

    public void pause() {
        Log.d("audio","pause");
        try {
            mAudioRecord.stop();
        }catch (Exception e){

        }
        isRecording = false;
    }


    public void stop() {
        Log.d("audio","stop");
        if (mAudioRecord != null) {
            mAudioRecord.stop();
//            mAudioRecord.release();
        }
        isRecording = false;
        dataBuffer.clear();
    }

    public void flush() {
        Log.d("audio","flush");
        if (isRecording) {
            stop();
        }

        if (dataEncodeThread != null) {

            Message msg = Message.obtain(dataEncodeThread.getHandler(),
                    DataEncodeThread.PROCESS_STOP);
            msg.sendToTarget();

            dataEncodeThread.flush();
        }

    }


    public boolean isRecording() {
        return isRecording;
    }


    /**
     * 计算声音的平均能级leavel.
     *
     * @param buffer   buffer
     * @param readSize readSize
     */
    private short calculateRealVolume(short[] buffer, int readSize) {
        double value = 0;
        int lenth = readSize / sampleRate;
        if (buffer.length <= 0 || readSize <= 0) {
            return 0;
        }

        for (int i = 0; i < lenth; i++) {
            value += Math.pow(buffer[i * sampleRate], 2);
        }

        return (short) Math.sqrt(value / lenth);
    }


    /**
     * listener.
     */
    public OnWaveChangeListener mOnWaveChangeListener;
    public OnStatuChangeListner mOnStatuChangeListner;


    public OnWaveChangeListener getOnWaveChangeListener() {
        return mOnWaveChangeListener;
    }

    public void setOnWaveChangeListener(OnWaveChangeListener onWaveChangeListener) {
        this.mOnWaveChangeListener = onWaveChangeListener;
    }

    public OnStatuChangeListner getOnStatuChangeListner() {
        return mOnStatuChangeListner;
    }

    public void setOnStatuChangeListner(OnStatuChangeListner onStatuChangeListner) {
        this.mOnStatuChangeListner = onStatuChangeListner;
    }

    /**
     * OnWaveChangeListener.
     */
    public interface OnWaveChangeListener {

        public void onChange(List<Short> data);

    }

    /**
     * OnStatuChangeListner.
     */
    interface OnStatuChangeListner {

        void onStart();

        void onPause();

        void onRestart();

        void onStop();

        void onError();
    }
}