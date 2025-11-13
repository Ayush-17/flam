#include "ImageProcessor.h"
#include <android/log.h>
#include <opencv2/imgproc.hpp>

#define LOG_TAG "ImageProcessor"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

ImageProcessor::ImageProcessor() : frameWidth(0), frameHeight(0) {
    LOGD("ImageProcessor created");
}

ImageProcessor::~ImageProcessor() {
    LOGD("ImageProcessor destroyed");
}

void ImageProcessor::processFrame(uint8_t* frameData, int width, int height, int rowStride) {
    if (frameData == nullptr) {
        LOGD("Frame data is null");
        return;
    }

    if (frameWidth != width || frameHeight != height) {
        frameWidth = width;
        frameHeight = height;

        processedMat = cv::Mat(height, width, CV_8UC1);
        LOGD("Allocated processed data buffer for %d x %d", width, height);
    }

    grayMat = cv::Mat(height, width, CV_8UC1, frameData, rowStride);
    grayMat.copyTo(processedMat);
}

uint8_t* ImageProcessor::getProcessedFrame() {
    if (processedMat.empty()) {
        return nullptr;
    }
    return processedMat.data;
}