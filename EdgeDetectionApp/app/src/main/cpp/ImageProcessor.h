#ifndef EDGEDETECTION_IMAGEPROCESSOR_H
#define EDGEDETECTION_IMAGEPROCESSOR_H

#include <cstdint>
#include <opencv2/core.hpp>

class ImageProcessor {
public:
    ImageProcessor();
    ~ImageProcessor();

    void processFrame(uint8_t* frameData, int width, int height, int rowStride);

    uint8_t* getProcessedFrame();

private:
    int frameWidth;
    int frameHeight;

    cv::Mat grayMat;
    cv::Mat processedMat;
};

#endif //EDGEDETECTION_IMAGEPROCESSOR_H