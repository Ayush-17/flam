#include <jni.h>
#include <android/log.h>
#include <cstdint>
#include "ImageProcessor.h"

#define LOG_TAG "NativeLib"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

static ImageProcessor* imageProcessor = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_edgedetection_MainActivity_initNative(JNIEnv* env, jobject thiz) {
    LOGD("initNative called");
    if (imageProcessor == nullptr) {
        imageProcessor = new ImageProcessor();
    }
}

JNIEXPORT void JNICALL
Java_com_example_edgedetection_MainActivity_destroyNative(JNIEnv* env, jobject thiz) {
    LOGD("destroyNative called");
    if (imageProcessor != nullptr) {
        delete imageProcessor;
        imageProcessor = nullptr;
    }
}

JNIEXPORT void JNICALL
Java_com_example_edgedetection_MainActivity_processFrameNative(
        JNIEnv* env,
        jobject thiz,
        jint width,
        jint height,
        jobject y_plane,
        jint y_stride) {
    auto* yBuffer = reinterpret_cast<uint8_t*>(env->GetDirectBufferAddress(y_plane));
    if (yBuffer == nullptr || imageProcessor == nullptr) {
        return;
    }

    imageProcessor->processFrame(yBuffer, width, height, y_stride);
}

JNIEXPORT void JNICALL
Java_com_example_edgedetection_GLView_onGlSurfaceCreated(JNIEnv* env, jobject thiz) {
    LOGD("JNI: onGlSurfaceCreated");
    if (imageProcessor != nullptr) {
        imageProcessor->initGl();
    }
}

JNIEXPORT void JNICALL
Java_com_example_edgedetection_GLView_onGlSurfaceChanged(JNIEnv* env, jobject thiz, jint width, jint height) {
    LOGD("JNI: onGlSurfaceChanged %dx%d", width, height);
    if (imageProcessor != nullptr) {
        imageProcessor->resizeGl(width, height);
    }
}

JNIEXPORT jfloat JNICALL
Java_com_example_edgedetection_GLView_onGlDrawFrame(JNIEnv* env, jobject thiz) {
    if (imageProcessor != nullptr) {
        return imageProcessor->drawGl();
    }
    return 0.0f;
}

}