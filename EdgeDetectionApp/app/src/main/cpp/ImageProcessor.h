#ifndef EDGEDETECTION_IMAGEPROCESSOR_H
#define EDGEDETECTION_IMAGEPROCESSOR_H

#include <cstdint>
#include <opencv2/core.hpp>
#include <GLES2/gl2.h>
#include "SimpleTextureRenderer.h"
#include <mutex>
#include <chrono>

class ImageProcessor {
public:
    ImageProcessor();
    ~ImageProcessor();

    void processFrame(uint8_t* frameData, int width, int height, int rowStride);

    void initGl();
    void resizeGl(int width, int height);
    float drawGl();

private:
    int frameWidth;
    int frameHeight;

    cv::Mat grayMat;
    cv::Mat processedMat;

    GLuint textureId;
    bool newFrameAvailable;
    int viewportWidth;
    int viewportHeight;

    SimpleTextureRenderer* renderer;
    std::mutex frameMutex;

    std::chrono::steady_clock::time_point lastFrameTime;
    float currentFps;
};

#endif //EDGEDETECTION_IMAGEPROCESSOR_H