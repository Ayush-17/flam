#ifndef EDGEDETECTION_IMAGEPROCESSOR_H
#define EDGEDETECTION_IMAGEPROCESSOR_H

#include <cstdint>
#include <opencv2/core.hpp>
#include <GLES2/gl2.h>
#include "SimpleTextureRenderer.h"

class ImageProcessor {
public:
    ImageProcessor();
    ~ImageProcessor();

    void processFrame(uint8_t* frameData, int width, int height, int rowStride);

    void initGl();
    void resizeGl(int width, int height);
    void drawGl();

    uint8_t* getProcessedFrame();

private:
    int frameWidth;
    int frameHeight;

    cv::Mat grayMat;
    cv::Mat processedMat;

    GLuint textureId;
    int viewportWidth;
    int viewportHeight;
    bool newFrameAvailable;

    SimpleTextureRenderer* renderer;
};

#endif //EDGEDETECTION_IMAGEPROCESSOR_H