//
// Created by LiangKeJin on 2024/7/28.
//

#pragma once

#include "BaseFilter.h"

NAMESPACE_WUTA

class FaceMorphFilter : public BaseFilter {
public:
    FaceMorphFilter() : BaseFilter("face_morph") {
        defAttribute("a_vertexCoord", DataType::FLOAT_POINTER)->bind(vertexCoord());
        defAttribute("a_srcTexCoord", DataType::FLOAT_POINTER)->bind(textureCoord());
        defAttribute("a_dstTexCoord", DataType::FLOAT_POINTER)->bind(m_dst_tex_coord);
        defUniform("srcImg", DataType::SAMPLER_2D);
        defUniform("dstImg", DataType::SAMPLER_2D);
        defUniform("alpha", DataType::FLOAT);
    }

public:
    void setSrcTexCoord(const float *p, int size) {
        textureCoord().set(p, size, GL_TRIANGLES, size/2);
    }

    void setSrcImg(const Texture& tex) {
        uniform("srcImg")->set(tex.id());
    }

    void setDstTexCoord(const float *p, int size) {
        m_dst_tex_coord.set(p, size, GL_TRIANGLES, size/2);
    }

    void setDstImg(const Texture& tex) {
        uniform("dstImg")->set(tex.id());
    }

    void setAlpha(float alpha) {
        uniform("alpha")->set(alpha);
    }

protected:
    const char *vertexShader() override {
#ifndef GLAPI
        return R"(
        attribute vec4 position;
        attribute vec2 inputTextureCoordinate;
        varying highp vec2 textureCoordinate;
        void main() {
            gl_Position = position;
            textureCoordinate = inputTextureCoordinate;
        })";
#else
        return R"(
        #version 330 core
        in vec4 a_vertexCoord;
        in vec2 a_srcTexCoord;
        in vec2 a_dstTexCoord;
        out highp vec2 srcTexCoord;
        out highp vec2 dstTexCoord;
        void main() {
            srcTexCoord = a_srcTexCoord;
            dstTexCoord = a_dstTexCoord;
            gl_Position = a_vertexCoord;
        })";
#endif
    }

    const char *fragmentShader() override {
#ifndef GLAPI
        return R"(
        varying highp vec2 textureCoordinate;
        uniform sampler2D inputImageTexture;
        uniform mediump float alpha;
        void main() {
            gl_FragColor = texture2D(inputImageTexture, textureCoordinate) * alpha;
        })";
#else
        return R"(
        #version 330 core
        in highp vec2 srcTexCoord;
        in highp vec2 dstTexCoord;
        uniform sampler2D srcImg;
        uniform sampler2D dstImg;

        uniform mediump float alpha;
        out vec4 fragColor;
        void main() {
            vec4 src = texture(srcImg, srcTexCoord);
            vec4 dst = texture(dstImg, dstTexCoord);
            vec4 fc = src * (1.0 - alpha) + dst * alpha;
            fragColor = vec4(fc.xyz, 1.0);
        })";
#endif
    }

private:
    int m_triangle_points = 0;
    TextureCoord m_dst_tex_coord;
};

NAMESPACE_END