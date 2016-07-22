package com.fuchao.ffmpegandroidplayer;

import android.provider.Settings;
import android.view.Surface;

/**
 * 视屏播放类
 * Created by fuchao on 16-7-22.
 */
public class VideoPlayer {

    private String filePath;//要播放的文件地址

    static {
        //加载JNI库
        System.loadLibrary("VideoPlayer");
    }

    public VideoPlayer(String filePath) {
        this.filePath = filePath;
    }

    //初始化播放器
    public native void init(Object surface);

    //播放
    public native int play();

    //暂停
    public native int pause();

    //停止
    public native int stop();

    //快进/快退
    public native int seek(long time);
}
