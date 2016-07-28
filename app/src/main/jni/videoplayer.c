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
    int i;
    for(i =0;i < pFormatCtx->nb_streams;i++)
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
    //获取视频的宽、高
    int videoWidth = pCodecCtx->width;
    int videoHeight = pCodecCtx->height;
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
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height,1);
    uint8_t *buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    //这个函数并没有真正的转换，我理解只是将一些图片的信息写到新的buffer中，然后将pFrameRGB的一些字段指向部分buffer的空间，为后面转换做准备,不然转换的时候不知道数据往哪里填。
    //pFrameRGB仅仅只是分配了一个结构体的空间，这里buff才是真正存储转换后的数据，也就是说pFrameRGB是指向buff的
    //英语原文是：Assign appropriate parts of buffer to image planes in pFrameRGB
    //翻译一下就是：为PFrameRGB中的图片分配适当的部分空间
    av_image_fill_arrays(pFrameRGB->data,pFrameRGB->linesize,buffer,AV_PIX_FMT_RGBA,pCodecCtx->width,pCodecCtx->height,1);
    //下面我们终于可以从流里面拿到数据了，注意每次循环读取一个包Package,帧是逻辑上的概念，实际数据是安照Packet的格式来存储的。类似于IP协议中的分包,ip包是逻辑概念，实际发送的是mac包
    //先获取一个转换用的上下文
    struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,pCodecCtx->width,pCodecCtx->height,AV_PIX_FMT_RGBA,SWS_BILINEAR,NULL,NULL,NULL);
    //下面开始循环读Packet了
    int frameFinished;
    AVPacket packet;
    //获取native widow.在循环之前，获取从java层传下来的surfaceview的缓冲区
    nativeWindow = ANativeWindow_fromSurface(env,surfaceObj);
    //设置native_window的buffer大小，可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow,videoWidth,videoHeight,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;
    while(av_read_frame(pFormatCtx, &packet) >= 0)
    {
        //判断这个包是不是视屏流的包,videoStream就是我们上面获取的流在结构体中的顺序
        if(packet.stream_index == videoStream)
        {
            //真正的获取原始数据,获取的原始数据放在pFrame所指向的缓冲区中
            //注意：解码函数就一个，没有avcodec_decode_video(),也没有avcodec_decode_video1(),这个函数名字有点坑
            avcodec_decode_video2(pCodecCtx,pFrame,&frameFinished, &packet);
            //这里有可能一个Frame不止一个package,所以用frameFinished这个变量来标示一个frame是否已经全部取到了.英文原文如下:
            //However, we might not have all the information we need for a frame after decoding a packet, so avcodec_decode_video() sets frameFinished for us when we have the next frame
            if(frameFinished)
            {
                //获取并锁住surfaceView的缓冲区
                ANativeWindow_lock(nativeWindow,&windowBuffer,0);
                //真正的将frame从原始格式转换为RGB格式.这里也可以看到pFrameRGB->data,pFrameRGB->linesize这些数据我们之前已经用av_image_fill_arrays()给分配好了
                sws_scale(sws_ctx,(uint8_t const * const *)pFrame->data,pFrame->linesize,0,pCodecCtx->height,pFrameRGB->data,pFrameRGB->linesize);
                //到这里我们已经拿到一个frame的视频数据，这个frame就是pFrameRGB,数据就存在pFrameRGB->data字段中,接下来就是把pFrameRGB中的数据复制到surfaceView的缓冲区windowBuffer中
                //但是这里要注意.复制的时候要一行一行的复制，因为当屏幕比图片大或小的时候，复制过去的数据就会乱
                uint8_t *dst = windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;//windowBUffer.strid是一行的像素数目,然后我们设置的是RGB_8888，所以一个像素占了4个字节，其中最高的一个字节表示透明度
                uint8_t *src = (uint8_t*)pFrameRGB->data[0];//data是个数组指针,也就是个二维数组
                int srcStride = pFrameRGB->linesize[0];//pFrameRGB->linesize保存的是frame每一行的宽度，单位是字节
                //开始一行一行的复制
                //LOGI("start copy data");
                int h;
                for(h = 0;h<videoHeight;h++)
                {
                    memcpy(dst + h * dstStride,src + h * srcStride,srcStride);
                }
                //画到屏幕上
                ANativeWindow_unlockAndPost(nativeWindow);
            }
        }
        //释放掉这个packet
        av_packet_unref(&packet);
    }
    //释放用于存储转换后的数据缓冲区
    av_free(buffer);
    //释放指向转换后数据的结构体
    av_free(pFrameRGB);
    //释放原始数据
    av_free(pFrame);
    //关闭解码器
    avcodec_close(pCodecCtx);
    //关闭视频文件
    avformat_close_input(&pFormatCtx);
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
