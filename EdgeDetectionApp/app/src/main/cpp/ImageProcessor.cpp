#include "ImageProcessor.h"
#include <android/log.h>
#include <opencv2/imgproc.hpp>
#include <GLES2/gl2ext.h>
#include <cstdlib>

#define LOG_TAG "ImageProcessor"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static const char* VERTEX_SHADER = R"(
    attribute vec4 a_position;
    attribute vec2 a_texCoord;
    varying vec2 v_texCoord;
    void main() {
        gl_Position = vec4(a_position.xy, 0.0, 1.0);
        v_texCoord = a_texCoord;
    }
)";

static const char* FRAGMENT_SHADER = R"(
    precision mediump float;
    varying vec2 v_texCoord;
    uniform sampler2D u_texture;
    void main() {
        float gray = texture2D(u_texture, v_texCoord).r;
        gl_FragColor = vec4(gray, gray, gray, 1.0);
    }
)";

ImageProcessor::ImageProcessor()
    : frameWidth(0),
      frameHeight(0),
      shaderProgram(0),
      textureId(0),
      attribPosition(-1),
      attribTexCoord(-1),
      uniformTexture(-1),
      viewportWidth(0),
      viewportHeight(0),
      newFrameAvailable(false) {
    LOGD("ImageProcessor created");
}

ImageProcessor::~ImageProcessor() {
    LOGD("ImageProcessor destroyed");
}

void ImageProcessor::processFrame(uint8_t* frameData, int width, int height, int rowStride) {
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

uint8_t* ImageProcessor::getProcessedFrame() {
    if (processedMat.empty()) {
        return nullptr;
    }
    return processedMat.data;
}

void ImageProcessor::initGl() {
    LOGD("initGl: Initializing OpenGL state");
    shaderProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (shaderProgram == 0) {
        LOGE("initGl: Failed to create shader program");
        return;
    }

    attribPosition = glGetAttribLocation(shaderProgram, "a_position");
    attribTexCoord = glGetAttribLocation(shaderProgram, "a_texCoord");
    uniformTexture = glGetUniformLocation(shaderProgram, "u_texture");

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    LOGD("initGl: Completed with texture %u", textureId);
}

void ImageProcessor::resizeGl(int width, int height) {
    viewportWidth = width;
    viewportHeight = height;
    glViewport(0, 0, width, height);
    LOGD("resizeGl: viewport %d x %d", width, height);
}

void ImageProcessor::drawGl() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (shaderProgram == 0 || textureId == 0) {
        return;
    }

    if (newFrameAvailable && !processedMat.empty()) {
        glActiveTexture(GL_TEXTURE0);
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
        newFrameAvailable = false;
        LOGD("drawGl: Uploaded new frame to texture");
    }

    glUseProgram(shaderProgram);

    static const GLfloat vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    static const GLfloat texCoords[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glEnableVertexAttribArray(attribPosition);
    glVertexAttribPointer(attribPosition, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    glEnableVertexAttribArray(attribTexCoord);
    glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

    glUniform1i(uniformTexture, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(attribPosition);
    glDisableVertexAttribArray(attribTexCoord);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint ImageProcessor::loadShader(GLenum type, const char* shaderSrc) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        LOGE("loadShader: glCreateShader failed");
        return 0;
    }

    glShaderSource(shader, 1, &shaderSrc, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = static_cast<char*>(std::malloc(static_cast<size_t>(infoLen)));
            if (infoLog != nullptr) {
                glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
                LOGE("loadShader: compile error %s", infoLog);
                std::free(infoLog);
            }
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint ImageProcessor::createProgram(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSrc);
    if (vertexShader == 0) {
        return 0;
    }

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program == 0) {
        LOGE("createProgram: glCreateProgram failed");
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = static_cast<char*>(std::malloc(static_cast<size_t>(infoLen)));
            if (infoLog != nullptr) {
                glGetProgramInfoLog(program, infoLen, nullptr, infoLog);
                LOGE("createProgram: link error %s", infoLog);
                std::free(infoLog);
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}