package com.fuchao.ffmpegandroidplayer;

import android.content.res.ObbInfo;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity {

    public static final String TAG = "VideoPlayer";
    public static final String PATH = "/mnt/sdcard/a.mp4";
    private SurfaceView surfaceView;
    private SurfaceHolder surfaceHolder;
    private VideoPlayer videoPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        this.surfaceView = (SurfaceView) findViewById(R.id.surface_view);
        this.surfaceHolder = surfaceView.getHolder();
        //创建视屏播放器
        this.videoPlayer = new VideoPlayer(PATH);
        this.surfaceHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                final SurfaceHolder holder = surfaceHolder;
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        //SurfaceView创建完毕后就可以开始播放了
                        videoPlayer.init(holder.getSurface());
                    }
                }).start();
            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
                //SurfaceView销毁时停止播放
                videoPlayer.stop();
            }
        });
    }
}
