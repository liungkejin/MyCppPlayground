//
// Created on 2024/6/5.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#pragma once

#include <Playground.h>

#if defined(__OS_HARMONY__)
#include <GLES3/gl3.h>
#elif defined(__ANDROID__)
#include <GLES3/gl3.h>
#else
#include <OpenGL/gl3.h>
#endif

NAMESPACE_WUTA

#define INVALID_GL_ID ((GLuint)(-1))

#define CHECK_GL_ERROR { GLenum en = glGetError(); if (en != GL_NO_ERROR) { _ERROR("find gl error: %d", en); }}

class GLUtil {
public:
    static GLuint loadShader(const char *str, int type) {
        GLuint shader = glCreateShader(type);
        _ERROR_RETURN_IF(shader == INVALID_GL_ID, INVALID_GL_ID, "GLUtil::loadShader create shader failed!");

        glShaderSource(shader, 1, &str, nullptr);
        glCompileShader(shader);

        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

            if (infoLen > 1) {
                char * infoLog = new char[infoLen+1];
                memset(infoLog, 0, infoLen+1);
                glGetShaderInfoLog(shader, infoLen, nullptr, (GLchar *)infoLog);
                _ERROR("Error compiling shader:%s\n%s", infoLog, str);
                delete [] infoLog;
            }

            glDeleteShader(shader);
            return INVALID_GL_ID;
        }

        return shader;
    }

    static GLuint loadProgram(const char *vstr, const char *fstr) {
        GLuint vertex = loadShader(vstr, GL_VERTEX_SHADER);
        _ERROR_RETURN_IF(vertex == INVALID_GL_ID, INVALID_GL_ID, "loadProgram vertex failed");

        GLuint fragment = loadShader(fstr, GL_FRAGMENT_SHADER);
        if (fragment == INVALID_GL_ID) {
            _ERROR("loadProgram fragment failed");
            glDeleteShader(vertex);
            return INVALID_GL_ID;
        }

        GLuint program = glCreateProgram();
        if (program == INVALID_GL_ID) {
            _ERROR("loadProgram: create program error");
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            return INVALID_GL_ID;
        }

        GLint linked;
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &linked);

        if (!linked) {
            _ERROR("loadProgram linked error");
            GLint infoLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                char * infoLog = new char[infoLen+1];
                memset(infoLog, 0, infoLen+1);
                glGetProgramInfoLog(program, infoLen, nullptr, (GLchar *)infoLog);
                _ERROR("Error linking program: %s", infoLog);
                delete [] infoLog;
            }
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            glDeleteProgram(program);
            return INVALID_GL_ID;
        }
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return program;
    }

    static void clearColor(float r, float g, float b, float a) {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);
    }
};
NAMESPACE_END
