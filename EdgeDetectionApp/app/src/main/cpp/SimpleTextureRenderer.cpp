#include "SimpleTextureRenderer.h"
#include <android/log.h>
#include <cstdlib>

#define LOG_TAG "SimpleTextureRenderer"
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

static const GLfloat VERTICES[] = {
    -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
     1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
    -1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 0.0f
};

static const GLushort INDICES[] = {
    0, 1, 2,
    2, 1, 3
};

SimpleTextureRenderer::SimpleTextureRenderer()
    : shaderProgram(0),
      attribPosition(-1),
      attribTexCoord(-1),
      uniformTexture(-1),
      vbo(0),
      ibo(0) {}

SimpleTextureRenderer::~SimpleTextureRenderer() {
    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (ibo != 0) {
        glDeleteBuffers(1, &ibo);
        ibo = 0;
    }
}

bool SimpleTextureRenderer::setup() {
    shaderProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (shaderProgram == 0) {
        LOGE("setup: failed to create shader program");
        return false;
    }

    attribPosition = glGetAttribLocation(shaderProgram, "a_position");
    attribTexCoord = glGetAttribLocation(shaderProgram, "a_texCoord");
    uniformTexture = glGetUniformLocation(shaderProgram, "u_texture");

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

void SimpleTextureRenderer::draw(GLuint textureId) {
    if (shaderProgram == 0 || vbo == 0 || ibo == 0) {
        return;
    }

    glUseProgram(shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(uniformTexture, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    const GLsizei stride = 5 * sizeof(GLfloat);
    glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(attribPosition);

    glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(attribTexCoord);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, reinterpret_cast<const void*>(0));

    glDisableVertexAttribArray(attribPosition);
    glDisableVertexAttribArray(attribTexCoord);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint SimpleTextureRenderer::loadShader(GLenum type, const char* shaderSrc) {
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

GLuint SimpleTextureRenderer::createProgram(const char* vertexSrc, const char* fragmentSrc) {
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