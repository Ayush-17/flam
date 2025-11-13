#ifndef EDGEDETECTION_IMAGEPROCESSOR_H
#define EDGEDETECTION_IMAGEPROCESSOR_H

#include <cstdint>

class ImageProcessor {
public:
    ImageProcessor();
    ~ImageProcessor();

    void processFrame(uint8_t* frameData, int width, int height);

    uint8_t* getProcessedFrame();

private:
    int frameWidth;
    int frameHeight;
    uint8_t* processedData;
};

#endif //EDGEDETECTION_IMAGEPROCESSOR_H