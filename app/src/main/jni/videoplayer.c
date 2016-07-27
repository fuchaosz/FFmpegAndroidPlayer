//实际播放类
// Created by fuchao on 16-7-22.
#include "com_fuchao_ffmpegandroidplayer_VideoPlayer.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <stdio.h>

//定义日志打印
#define  LOG_TAG    "VideoPlayer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//定义全局的文件地址变量
char *path = NULL;
AVFormatContext *pFormatCtx = NULL;//ffmpeg的上下文变量
AVCodecContext *pCodecCtx = NULL;//编解码的上下文环境
AVCodec *pCodec = NULL;//视频编解码器
ANativeWindow *nativeWindow = NULL;//本地窗口

/*
 * Class:     com_fuchao_ffmpegandroidplayer_VideoPlayer
 * Method:    init
 * Signature: (Ljava/lang/Object;)V
 */
JNIEXPORT jint JNICALL Java_com_fuchao_ffmpegandroidplayer_VideoPlayer_init
        (JNIEnv *env, jobject jobj, jobject surfaceObj)
{
    LOGE("Hello world");
    //取出文件路径
    jclass cls = (*env)->GetObjectClass(env, jobj);
    jfieldID fid = (*env)->GetFieldID(env, cls, "filePath", "Ljava/lang/String;");
    jstring file_path = (jstring) (*env)->GetObjectField(env, jobj, fid);
    path = (*env)->GetStringUTFChars(env, file_path, NULL);
    if (path == NULL) {
        LOGE("获取java中的path失败");
        return -1;
    }
    else
    {
        //输出要播放的文件的地址
        LOGE("%s", path);
    }
    //AVCode *codec = NULL;
     //注册所有类型的格式
    av_register_all();
    //获取FFmpeg的上下文变量
    pFormatCtx = avformat_alloc_context();
    //打开文件
    if(avformat_open_input(&pFormatCtx,path, NULL, NULL) != 0)
    {
        LOGE("Could not open file: %s\n",path);
        return -1;
    }
    //获取流信息
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0)
    {
        LOGE("Could not find stream infomation");
        return -1;
    }
    //找出流中的第一个视频流
    int videoStream = -1;
    for(int i=0;i < pFormatCtx->nb_streams;i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && videoStream < 0)
        {
            videoStream = i;
        }
    }
    if(videoStream == -1)
    {
        LOGE("Didn't find a video stream");
        return -1;
    }
    //获取视频流的编码上下文环境
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    //获取视频的编解码器
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL)
    {
        LOGE("Codec not found.");
        return -1;
    }
    //打开解码器
    if(avcodec_open2(pCodecCtx,pCodec,NULL) < 0)
    {
        LOGE("Could not open codec.");
        return -1;
    }
    //获取native widow
    nativeWindow = ANativeWindow_fromSurface(env,surfaceObj);
    //获取视频的宽、高
    int videoWidth = pCodecCtx->width;
    int viedeoHeight = pCodecCtx->height;
    //设置native_window的buffer大小，可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow,videoWidth,videoHeight,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;
    //申请一块空间，用于保存原始数据
    AVFrame *pFrame = av_frame_alloc();
    //申请一块空间，用于保存转换为RGB后的数据,流Stream是由帧Frame组成的
    AVFrame *pFrameRGB = av_frame_alloc();
    if(pFrameRGB == NULL)
    {
        LOGE("Could not allocate video frame");
        return -1;
    }
    //计算机的图片是以RGB格式存储的，所以我们要把Frame从它本地格式转换会24位的RGB格式
    int numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
    uint8_t *buffer = (unit8_t)av_malloc(numBytes * sizeof(unit8_t));
    //这个函数并没有真正的转换，我理解只是将一些图片的信息写到新的buffer中，为后面转换做准备
    //英语原文是：Assign appropriate parts of buffer to image planes in pFrameRGB
    //翻译一下就是：为PFrameRGB中的图片分配适当的部分空间
    av_image_fill_arrays(pFrameRGB->data,pFrameRGB->linesize,buffer,AV_PIX_FMT_RGB,pCodecCtx->width,pCodecCtx->height,1);
    //下面我们终于可以从流里面拿到数据了，注意每次循环读取一个包Package,帧是逻辑上的概念，实际数据是安照Packet的格式来存储的
    //先获取一个转换用的上下文
    struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,pCodexCtx->width,pCodecCtx->height,PIX_FMT_RGB24,SWS_BILINEAR,NULL,NULL,NULL);
    //下面开始循环读Packet了
    int frameFinished;
    int i = 0;
    AVPacket packet;
    while(av_read_frame(pFormatCtx, &packet) >= 0)
    {
        //判断这个包是不是视屏流的包,videoStream就是我们上面获取的流在结构体中的顺序
        if(packet.stream_index == videoStream)
        {
            //真正的获取原始数据
            avcodec_decode_video2(pCodecCtx,pFrame,&frameFinished, &packet);
            //判断我们收到的包是不是视频数据包，有可能视屏流中带有其他信息的包
            if(frameFinished)
            {
                //真正的将frame从原始格式转换为RGB格式
                sws_scale();
            }
        }

    }



    //删除局部引用
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, file_path);
    return 0;
}

/*
 * Class:     com_fuchao_ffmpegandroidplayer_VideoPlayer
 * Method:    play
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_fuchao_ffmpegandroidplayer_VideoPlayer_play
        (JNIEnv *env, jobject jobj)
{
    return 1;
}

/*
 * Class:     com_fuchao_ffmpegandroidplayer_VideoPlayer
 * Method:    pause
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_fuchao_ffmpegandroidplayer_VideoPlayer_pause
        (JNIEnv *env, jobject jobj)
{
    return 0;
}

/*
 * Class:     com_fuchao_ffmpegandroidplayer_VideoPlayer
 * Method:    stop
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_fuchao_ffmpegandroidplayer_VideoPlayer_stop
        (JNIEnv *env, jobject jobj)
{
    return 0;
}

/*
 * Class:     com_fuchao_ffmpegandroidplayer_VideoPlayer
 * Method:    seek
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_com_fuchao_ffmpegandroidplayer_VideoPlayer_seek
        (JNIEnv *env, jobject jobj, jlong time)
{
    return 0;
}
