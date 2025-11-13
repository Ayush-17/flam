#include "ImageProcessor.h"
#include <android/log.h>
#include <opencv2/imgproc.hpp>
#include <GLES2/gl2ext.h>

#define LOG_TAG "ImageProcessor"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

ImageProcessor::ImageProcessor()
    : frameWidth(0),
      frameHeight(0),
      textureId(0),
      newFrameAvailable(false),
      viewportWidth(0),
      viewportHeight(0),
      renderer(nullptr),
      currentFps(0.0f) {
    LOGD("ImageProcessor created");
}

ImageProcessor::~ImageProcessor() {
    LOGD("ImageProcessor destroyed");
    if (renderer != nullptr) {
        delete renderer;
        renderer = nullptr;
    }
    if (textureId != 0) {
        glDeleteTextures(1, &textureId);
        textureId = 0;
    }
}

void ImageProcessor::processFrame(uint8_t* frameData, int width, int height, int rowStride) {
    std::lock_guard<std::mutex> lock(frameMutex);

    if (frameData == nullptr) {
        LOGE("processFrame: frameData is null");
        return;
    }

    if (frameWidth != width || frameHeight != height) {
        frameWidth = width;
        frameHeight = height;
        processedMat = cv::Mat(height, width, CV_8UC1);
        LOGD("Allocated processed data buffer for %d x %d", width, height);
    }

    grayMat = cv::Mat(height, width, CV_8UC1, frameData, rowStride);
    cv::Canny(grayMat, processedMat, 50, 100);

    newFrameAvailable = true;
}

void ImageProcessor::initGl() {
    LOGD("initGl: Initializing OpenGL");

    if (renderer == nullptr) {
        renderer = new SimpleTextureRenderer();
        if (!renderer->setup()) {
            LOGE("initGl: Failed to set up SimpleTextureRenderer");
            delete renderer;
            renderer = nullptr;
            return;
        }
    }

    if (textureId == 0) {
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    lastFrameTime = std::chrono::steady_clock::now();
    currentFps = 0.0f;
    LOGD("initGl: Texture ID %u", textureId);
}

void ImageProcessor::resizeGl(int width, int height) {
    viewportWidth = width;
    viewportHeight = height;
    glViewport(0, 0, width, height);
    LOGD("resizeGl: viewport %d x %d", width, height);
}

float ImageProcessor::drawGl() {
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime).count() / 1'000'000.0f;
    lastFrameTime = now;
    if (deltaTime > 0.0f) {
        currentFps = (currentFps * 0.9f) + ((1.0f / deltaTime) * 0.1f);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if (renderer == nullptr || textureId == 0) {
        return currentFps;
    }

    {
        std::lock_guard<std::mutex> lock(frameMutex);
        if (newFrameAvailable && !processedMat.empty()) {
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_LUMINANCE,
                processedMat.cols,
                processedMat.rows,
                0,
                GL_LUMINANCE,
                GL_UNSIGNED_BYTE,
                processedMat.data);
            glBindTexture(GL_TEXTURE_2D, 0);
            newFrameAvailable = false;
        }
    }

    renderer->draw(textureId);
    return currentFps;
}