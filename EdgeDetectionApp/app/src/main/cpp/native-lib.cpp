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
Java_com_example_edgedetection_MainActivity_processFrameNative(
        JNIEnv* env,
        jobject thiz,
        jint width,
        jint height,
        jobject y_plane,
        jint y_stride) {
    auto* yBuffer = reinterpret_cast<uint8_t*>(env->GetDirectBufferAddress(y_plane));
    if (yBuffer == nullptr) {
        LOGD("Failed to get Y-plane buffer address");
        return;
    }

    if (imageProcessor != nullptr) {
        imageProcessor->processFrame(yBuffer, width, height, y_stride);
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

}