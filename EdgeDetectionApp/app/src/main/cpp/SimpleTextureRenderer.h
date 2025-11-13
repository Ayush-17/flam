#ifndef EDGEDETECTION_SIMPLETEXTURERENDERER_H
#define EDGEDETECTION_SIMPLETEXTURERENDERER_H

#include <GLES2/gl2.h>

class SimpleTextureRenderer {
public:
    SimpleTextureRenderer();
    ~SimpleTextureRenderer();

    bool setup();
    void draw(GLuint textureId);

private:
    GLuint shaderProgram;
    GLint attribPosition;
    GLint attribTexCoord;
    GLint uniformTexture;
    GLuint vbo;
    GLuint ibo;

    GLuint loadShader(GLenum type, const char* shaderSrc);
    GLuint createProgram(const char* vertexSrc, const char* fragmentSrc);
};

#endif //EDGEDETECTION_SIMPLETEXTURERENDERER_H