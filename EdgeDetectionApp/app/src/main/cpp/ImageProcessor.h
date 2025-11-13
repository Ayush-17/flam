#ifndef EDGEDETECTION_IMAGEPROCESSOR_H
#define EDGEDETECTION_IMAGEPROCESSOR_H

#include <cstdint>
#include <opencv2/core.hpp>
#include <GLES2/gl2.h>

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

    GLuint shaderProgram;
    GLuint textureId;
    GLint attribPosition;
    GLint attribTexCoord;
    GLint uniformTexture;
    int viewportWidth;
    int viewportHeight;
    bool newFrameAvailable;

    GLuint loadShader(GLenum type, const char* shaderSrc);
    GLuint createProgram(const char* vertexSrc, const char* fragmentSrc);
};

#endif //EDGEDETECTION_IMAGEPROCESSOR_H