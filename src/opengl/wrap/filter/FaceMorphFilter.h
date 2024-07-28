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
    void setVertexCoord(const float *p, int size) override {
        BaseFilter::setVertexCoord(p, size);
        m_triangle_points = size / 2;
    }

    void setSrcTexCoord(const float *p, int size) {
        textureCoord().set(p, size);
    }

    void setSrcImg(const Texture2D& tex) {
        uniform("srcImg")->set(tex.id());
    }

    void setDstTexCoord(const float *p, int size) {
        m_dst_tex_coord.set(p, size);
    }

    void setDstImg(const Texture2D& tex) {
        uniform("dstImg")->set(tex.id());
    }

    void setAlpha(float alpha) {
        uniform("alpha")->set(alpha);
    }

protected:
    const char *vertexShader() override {
#ifdef GL_ES
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
#ifdef GL_ES
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

    void onDrawArrays() override {
        if (m_triangle_points > 0) {
            glDrawArrays(GL_TRIANGLES, 0, m_triangle_points);
        } else {
            BaseFilter::onDrawArrays();
        }
    }
private:
    int m_triangle_points = 0;
    TextureCoord m_dst_tex_coord;
};

NAMESPACE_END