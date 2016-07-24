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

/*
 * Class:     com_fuchao_ffmpegandroidplayer_VideoPlayer
 * Method:    init
 * Signature: (Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_fuchao_ffmpegandroidplayer_VideoPlayer_init
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
    }
    else {
        //输出要播放的文件的地址
        LOGE("%s", path);
    }
    //AVCode *codec = NULL;
    //注册所有类型的格式
    //av_register_all();
    //删除局部引用
    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, file_path);
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
