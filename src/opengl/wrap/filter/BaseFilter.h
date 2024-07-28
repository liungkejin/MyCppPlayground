//
// Created on 2024/6/30.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".


#pragma once
#include "../Framebuffer.h"
#include "../GLCoord.h"
#include "../Program.h"
#include "../Viewport.h"

NAMESPACE_WUTA

class BaseFilter {
public:
    explicit BaseFilter(const char *name) : m_name(name) {}

    Attribute *attribute(const char *name) { return m_program.attribute(name); }

    Uniform *uniform(const char *name) { return m_program.uniform(name); }

    const Viewport &viewport() const { return m_viewport; }

    void setViewport(const Viewport &viewport) { m_viewport.set(viewport); }

    void setViewport(int width, int height, bool scissor = false) { m_viewport.set(width, height, scissor); }

    void setViewport(int x, int y, int width, int height, bool scissor = false) {
        m_viewport.set(x, y, width, height, scissor);
    }

    virtual void setVertexCoord(const GLRect &rect, float viewW, float viewH) {
        m_vertex_coords.setByGLRect(viewW, viewH, rect);
    }

    virtual void setVertexCoord(const float *ps, int size) {
        m_vertex_coords.set(ps, size);
    }

    virtual void setTextureCoord(const GLRect &rect, float texW, float texH) {
        m_texture_coords.setByGLRect(texW, texH, rect);
    }

    // rotation 只支持 0, 90, 180, 270
    virtual void setTextureCoord(int rotation, bool flipH, bool flipV) {
        m_texture_coords.setFullCoord(rotation, flipH, flipV);
    }

    virtual void setTextureCoord(const float *ps, int size) {
        m_texture_coords.set(ps, size);
    }

    void render(Framebuffer *output = nullptr) {
        if (!m_program.valid()) {
            if (!m_program.create(vertexShader(), fragmentShader())) {
                return;
            }
            onProgramCreated();
        }

        onViewport();

        if (output) {
            output->bind();
        }

        if (!m_program.attach()) {
            _ERROR("Couldn't attach filter(%s) program", m_name.c_str());
            return;
        }

        onRender(output);
        onDrawArrays();

        if (output) {
            output->unbind();
        }

        onPostRender(output);
        m_program.detach();
    }

    virtual void release() { m_program.release(); }

protected:
    virtual const char *vertexShader() = 0;

    virtual const char *fragmentShader() = 0;

    Program& program() { return m_program; }

    Attribute *defAttribute(const char *name, DataType type) { return m_program.defAttribute(name, type); }

    Uniform *defUniform(const char *name, DataType type) { return m_program.defUniform(name, type); }

    VertexCoord &vertexCoord() { return m_vertex_coords; }

    TextureCoord &textureCoord() { return m_texture_coords; }

    virtual void onProgramCreated() {}

    virtual void onViewport() { m_viewport.apply(); }

    virtual void onRender(Framebuffer *output) { m_program.input(); }

    virtual void onDrawArrays() { glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }

    virtual void onPostRender(Framebuffer *output) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    //    void drawTriangleStrip(int count) { glDrawArrays(GL_TRIANGLE_STRIP, 0, count); }
    //
    //    void drawTriangleStrip(int offset, int count) { glDrawArrays(GL_TRIANGLE_STRIP, offset, count); }
    //
    //    void drawTriangle(int count) { glDrawArrays(GL_TRIANGLES, 0, count); }
    //
    //    void drawTriangle(int offset, int count) { glDrawArrays(GL_TRIANGLES, offset, count); }

private:
    const std::string m_name;

    Viewport m_viewport = Viewport(0, 0);
    Program m_program;

    VertexCoord m_vertex_coords;
    TextureCoord m_texture_coords;
};

NAMESPACE_END
