#include "ImageProcessor.h"
#include <android/log.h>
#include <cstring>

#define LOG_TAG "ImageProcessor"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

ImageProcessor::ImageProcessor() : frameWidth(0), frameHeight(0), processedData(nullptr) {
    LOGD("ImageProcessor created");
}

ImageProcessor::~ImageProcessor() {
    LOGD("ImageProcessor destroyed");
    if (processedData) {
        delete[] processedData;
        processedData = nullptr;
    }
}

void ImageProcessor::processFrame(uint8_t* frameData, int width, int height) {
    if (frameData == nullptr) {
        LOGD("Frame data is null");
        return;
    }

    if (frameWidth != width || frameHeight != height) {
        frameWidth = width;
        frameHeight = height;

        if (processedData) {
            delete[] processedData;
            processedData = nullptr;
        }

        processedData = new uint8_t[static_cast<size_t>(width) * static_cast<size_t>(height)];
        LOGD("Allocated processed data buffer for %d x %d", width, height);
    }

    if (processedData) {
        std::memcpy(processedData, frameData, static_cast<size_t>(width) * static_cast<size_t>(height));
    }
}

uint8_t* ImageProcessor::getProcessedFrame() {
    return processedData;
}