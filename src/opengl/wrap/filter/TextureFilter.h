//
// Created on 2024/7/5.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".


#pragma once
#include "BaseFilter.h"

NAMESPACE_WUTA

class TextureFilter : public BaseFilter {
public:
    TextureFilter() : BaseFilter("texture_filter") {
        defAttribute("position", DataType::FLOAT_POINTER)->bind(vertexCoord());
        defAttribute("inputTextureCoordinate", DataType::FLOAT_POINTER)->bind(textureCoord());
        defUniform("inputImageTexture", DataType::SAMPLER_2D);
        defUniform("alpha", DataType::FLOAT)->set(1.0f);
    }

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
        in vec4 position;
        in vec2 inputTextureCoordinate;
        out highp vec2 textureCoordinate;
        void main() {
            textureCoordinate = inputTextureCoordinate;
            gl_Position = position;
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
        in highp vec2 textureCoordinate;
        uniform sampler2D inputImageTexture;
        uniform mediump float alpha;
        out vec4 fragColor;
        void main() {
            fragColor = texture(inputImageTexture, textureCoordinate) * alpha;
        })";
#endif
    }

    TextureFilter &inputTexture(int id) {
        uniform("inputImageTexture")->set(id);
        return *this;
    }

    TextureFilter &inputTexture(const Texture &texture) {
        uniform("inputImageTexture")->set((int)texture.id());
        return *this;
    }

    TextureFilter &blend(bool enable) {
        m_blend = enable;
        return *this;
    }

    void onRender(Framebuffer *output) override {
        if (m_blend) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }
        BaseFilter::onRender(output);
    }

    void onPostRender(Framebuffer *output) override {
        BaseFilter::onPostRender(output);
        if (m_blend) {
            glDisable(GL_BLEND);
        }
    }

private:
    bool m_blend = false;
};

NAMESPACE_END
