//
// Created by LiangKeJin on 2024/7/28.
//

#pragma once

#include <Playground.h>
#include "wrap/filter/FaceMorphFilter.h"
#include "utils/Delaunator.h"
#include "opengl/wrap/filter/TextureFilter.h"

NAMESPACE_WUTA

/**
 * 思路：
 * 1. 两张图片不需要尺寸一致，只需要最后渲染的三角形
 */

class MorphImage {
public:
    void setImg(const uint8_t *data, int width, int height, GLenum format) {
        m_width = width;
        m_height = height;
        m_img.set(data, width, height, format);
        m_texture = INVALID_TEXTURE;
    }

    void setTexture(int id, int width, int height) {
        m_width = width;
        m_height = height;
        m_img.release();
        m_texture = Texture(id, width, height);
    }

    /**
     * 3 个关键点 左右眼球中心点，鼻子中心点的索引
     */
    void setKeyPoints(const std::vector<float> &points, int leftEye, int rightEye, int nose) {
        m_raw_points = points;
        m_left_eye = leftEye;
        m_right_eye = rightEye;
        m_nose = nose;
    }

    const Texture& rawTexture() {
        return m_texture.valid() ? m_texture : m_img.textureNonnull();
    }

    inline int width() const {
        return m_width;
    }

    inline int height() const {
        return m_height;
    }

    inline float px(int index) const {
        return m_raw_points[index * 2];
    }

    inline float py(int index) const {
        return m_raw_points[index * 2 + 1];
    }

    /**
     * 将图片和点都统一到 dw, dh 大小
     */
    Texture prepare(TextureFilter &texFilter, int dw, int dh) {
        m_trans_points.clear();
        Texture tex = rawTexture();
        int sw = m_width, sh = m_height;
        if (sw * dh != sh * dw) {
            GLRect rect(0, 0, sw, sh);
            float scale = std::min((float) dw / sw, (float) dh / sh);
            rect.scale(scale, scale);
            float ssw = sw * scale, ssh = sh * scale;
            float dx = dw / 2.f - ssw / 2.f, dy = dh / 2.f - ssh / 2.f;
            rect.translate(dx, dy);

            for (int i = 0, size = m_raw_points.size(); i < size; i += 2) {
                m_trans_points.push_back(m_raw_points[i] * scale + dx);
                m_trans_points.push_back(m_raw_points[i + 1] * scale + dy);
            }

            m_fb.create(dw, dh);
            Viewport viewport(dw, dh);
            viewport.enableClearColor(0, 0, 0, 1);
            texFilter.setViewport(viewport);
            texFilter.inputTexture(tex);
            texFilter.setVertexCoord(rect, dw, dh);
            texFilter.render(&m_fb);

            return m_fb.textureNonnull();
        } else {
            float scale = std::min((float) dw / sw, (float) dh / sh);
            for (int i = 0, size = m_raw_points.size(); i < size; i += 2) {
                m_trans_points.push_back(m_raw_points[i] * scale);
                m_trans_points.push_back(m_raw_points[i + 1] * scale);
            }

            return tex;
        }
    }

    void transformTo(MorphImage& dst, float percent) {
        float sumScale = dst.eyeDistance() / eyeDistance();
        float sumRotate = dst.eyeAngle() - eyeAngle();
        float sumTranX = dst.eyeCenterX() - eyeCenterX();
        float sumTranY = dst.eyeCenterY() - eyeCenterY();

        _INFO("sumScale: %.2f, sumRotate: %.2f, sumTranX: %.2f, sumTranY: %.2f", sumScale, sumRotate, sumTranX, sumTranY);
        float scale = 1 + (sumScale - 1) * percent;
        float rotate = sumRotate * percent;
        float tranX = sumTranX * percent;
        float tranY = sumTranY * percent;
        transformPoints(scale, rotate, tranX, tranY);

        float dstScale = 1.0f / sumScale + (1 - 1.0f / sumScale) * percent;
        float dstRotate = -sumRotate * (1 - percent);
        float dstTranX = sumTranX * (1 - percent);
        float dstTranY = sumTranY * (1 - percent);
        dst.transformPoints(dstScale, dstRotate, dstTranX, dstTranY);
    }

private:
    void transformPoints(float scale, float rotate, float dx, float dy) {
        for (int i = 0, size = m_trans_points.size(); i < size; i += 2) {
            m_trans_points[i] = m_trans_points[i] * scale + dx;
            m_trans_points[i+1] = m_trans_points[i+1] * scale + dy;
        }
    }

    float eyeDistance() {
        float lx = m_trans_points[m_left_eye*2];
        float ly = m_trans_points[m_left_eye*2+1];
        float rx = m_trans_points[m_right_eye*2];
        float ry = m_trans_points[m_right_eye*2+1];
        return sqrt((lx - rx) * (lx - rx) + (ly - ry) * (ly - ry));
    }

    float eyeCenterX() {
        float lx = m_trans_points[m_left_eye*2];
        float rx = m_trans_points[m_right_eye*2];
        return (lx + rx) / 2.f;
    }

    float eyeCenterY() {
        float ly = m_trans_points[m_left_eye*2+1];
        float ry = m_trans_points[m_right_eye*2+1];
        return (ly + ry) / 2.f;
    }

    // 计算眼睛和 x 轴的夹角，顺时针，注意判断鼻子的方位
    float eyeAngle() {
        float lx = m_trans_points[m_left_eye*2];
        float ly = m_trans_points[m_left_eye*2+1];
        float rx = m_trans_points[m_right_eye*2];
        float ry = m_trans_points[m_right_eye*2+1];

        float nx = m_trans_points[m_nose*2];
        float ny = m_trans_points[m_nose*2+1];

        float dx = rx - lx;
        float dy = ry - ly;
        float angle = atan2(dy, dx);
        return angle;
    }

private:
    int m_width = 0;
    int m_height = 0;
    ImageTexture m_img;
    Texture m_texture = INVALID_TEXTURE;
    std::vector<float> m_raw_points;
    std::vector<float> m_trans_points;
    int m_left_eye = 0;
    int m_right_eye = 0;
    int m_nose = 0;

    Framebuffer m_fb;
};

class GLFaceMorph {
public:
    static void test(int width, int height, float percent);

public:
    void setSrcImg(const uint8_t *data, int width, int height, GLenum format, const std::vector<float> &points) {
        m_src_img.set(data, width, height, format);
        m_src_texture = INVALID_TEXTURE;
        m_src_raw_points = points;
    }

    void setSrcTexture(int id, int width, int height, const std::vector<float> &points) {
        m_src_img.release();
        m_src_texture = Texture(id, width, height);
        m_src_raw_points = points;
    }

    /**
     * 3 个关键点 左右眼球中心点，鼻子中心点
     */
    void setSrcKeyPointsIndex(int leftEye, int rightEye, int nose) {

    }

    void setDstImg(const uint8_t *data, int width, int height, GLenum format, const std::vector<float> &points) {
        m_dst_img.set(data, width, height, format);
        m_dst_texture = INVALID_TEXTURE;
        m_dst_raw_points = points;
    }

    void setDstTexture(int id, int width, int height, const std::vector<float> &points) {
        m_dst_img.release();
        m_dst_texture = Texture(id, width, height);
        m_dst_raw_points = points;
    }

    Framebuffer &render(float percent) {
        m_morph_filter.setAlpha(percent);
        const Texture &srcTex = srcTexture();
        const Texture &dstTex = dstTexture();

        int sw = srcTex.width(), sh = srcTex.height();
        int dw = dstTex.width(), dh = dstTex.height();
        transformSrc(srcTex, sw, sh, dw, dh);
        transformDst(dstTex, sw, sh, dw, dh);

        if (m_src_raw_points.size() < 2 || m_src_raw_points.size() != m_dst_raw_points.size()) {
            // 简单混合
            m_morph_filter.setVertexCoord(nullptr, 0);
            m_morph_filter.setSrcTexCoord(nullptr, 0);
            m_morph_filter.setDstTexCoord(nullptr, 0);
        } else {
            generateTriangles(dw, dh, percent);

            float *weight = m_percent_triangle_points.obtain<float>(m_triangle_pcount);
            for (int i = 0; i < m_triangle_pcount; ++i) {
                float a = m_src_triangle_points.at<float>(i);
                float b = m_dst_triangle_points.at<float>(i);
                weight[i] = ((1 - percent) * a + percent * b) * 2 - 1;
            }

            float *src = m_src_triangle_points.obtain<float>(m_triangle_pcount);
            float *dst = m_dst_triangle_points.obtain<float>(m_triangle_pcount);
            m_morph_filter.setVertexCoord(weight, m_triangle_pcount);
            m_morph_filter.setSrcTexCoord(src, m_triangle_pcount);
            m_morph_filter.setDstTexCoord(dst, m_triangle_pcount);
        }

        m_output_fb.create(dw, dh);
        m_morph_filter.setViewport(dw, dh);
        m_morph_filter.render(&m_output_fb);

        return m_output_fb;
    }

    void release() {
        // TODO
    }

private:
    const Texture &srcTexture() {
        return m_src_texture.valid() ? m_src_texture : m_src_img.textureNonnull();
    }

    const Texture &dstTexture() {
        return m_dst_texture.valid() ? m_dst_texture : m_dst_img.textureNonnull();
    }

    void transformSrc(const Texture &srcTex, int sw, int sh, int dw, int dh) {
        m_src_trans_points.clear();
        if (sw * dh != sh * dw) {
            // 先将 src 渲染到和 dst 一样尺寸的 framebuffer 上
            // fit center 渲染
            m_src_rect.setRect(0, 0, sw, sh);
            float scale = std::min((float) dw / sw, (float) dh / sh);
            m_src_rect.scale(scale, scale);
            float ssw = sw * scale, ssh = sh * scale;
            float dx = dw / 2.f - ssw / 2.f, dy = dh / 2.f - ssh / 2.f;
            m_src_rect.translate(dx, dy);

            for (int i = 0, size = m_src_raw_points.size(); i < size; i += 2) {
                m_src_trans_points.push_back(m_src_raw_points[i] * scale + dx);
                m_src_trans_points.push_back(m_src_raw_points[i + 1] * scale + dy);
            }

            m_src_fb.create(dw, dh);
            Viewport viewport(dw, dh);
            viewport.enableClearColor(0, 0, 0, 1);
            m_texture_filter.setViewport(viewport);
            m_texture_filter.inputTexture(srcTex);
            m_texture_filter.setVertexCoord(m_src_rect, dw, dh);
            m_texture_filter.render(&m_src_fb);

            m_morph_filter.setSrcImg(m_src_fb.textureNonnull());
        } else {
            m_morph_filter.setSrcImg(srcTex);
        }
    }

    void transformDst(const Texture &dstTex, int sw, int sh, int dw, int dh) {
        m_dst_trans_points.clear();
        for (int i = 0, size = m_dst_raw_points.size(); i < size; i+=2) {
            m_dst_trans_points.push_back(m_dst_raw_points[i]);
            m_dst_trans_points.push_back(m_dst_raw_points[i + 1]);
        }
        m_morph_filter.setDstImg(dstTex);
    }

    void generateTriangles(float width, float height, float percent) {
        {
            for (int i = 0, size = m_src_trans_points.size(); i < size; i += 2) {
                m_src_trans_points[i] = m_src_trans_points[i] / width;
                m_src_trans_points[i + 1] = m_src_trans_points[i + 1] / height;
            }
            m_src_trans_points.push_back(0);
            m_src_trans_points.push_back(0);
            m_src_trans_points.push_back(1);
            m_src_trans_points.push_back(0);
            m_src_trans_points.push_back(0);
            m_src_trans_points.push_back(1);
            m_src_trans_points.push_back(1);
            m_src_trans_points.push_back(1);
        }

        {
            for (int i = 0, size = m_dst_trans_points.size(); i < size; i += 2) {
                m_dst_trans_points[i] = m_dst_trans_points[i] / width;
                m_dst_trans_points[i + 1] = m_dst_trans_points[i + 1] / height;
            }
            m_dst_trans_points.push_back(0);
            m_dst_trans_points.push_back(0);
            m_dst_trans_points.push_back(1);
            m_dst_trans_points.push_back(0);
            m_dst_trans_points.push_back(0);
            m_dst_trans_points.push_back(1);
            m_dst_trans_points.push_back(1);
            m_dst_trans_points.push_back(1);
        }

        m_average_points.clear();
        for (int i = 0, size = m_src_trans_points.size(); i < size; ++i) {
            m_average_points.push_back((m_src_trans_points[i] + m_dst_trans_points[i]) / 2.);
        }

        // 生成三角形
        delaunator::Delaunator dela(m_average_points);

        m_triangle_pcount = (int) dela.triangles.size() * 2;
        float *src = m_src_triangle_points.obtain<float>(m_triangle_pcount);
        float *dst = m_dst_triangle_points.obtain<float>(m_triangle_pcount);
        int i = 0;
        for (size_t &t: dela.triangles) {
            src[i] = m_src_trans_points[t * 2];
            src[i + 1] = m_src_trans_points[t * 2 + 1];
            dst[i] = m_dst_trans_points[t * 2];
            dst[i + 1] = m_dst_trans_points[t * 2 + 1];
            i += 2;
        }
//        _INFO("generate triangle success! triangles: %d, i: %d", m_triangle_pcount, i);
    }

private:
    ImageTexture m_src_img;
    Texture m_src_texture = INVALID_TEXTURE;
    std::vector<float> m_src_raw_points;
    std::vector<float> m_src_trans_points;
    Framebuffer m_src_fb;
    GLRect m_src_rect;

    Array m_src_triangle_points;

    ImageTexture m_dst_img;
    Texture m_dst_texture = INVALID_TEXTURE;
    std::vector<float> m_dst_raw_points;
    std::vector<float> m_dst_trans_points;
    Framebuffer m_dst_fb;

    Array m_dst_triangle_points;

    int m_triangle_pcount = 0;

    Array m_percent_triangle_points;

    std::vector<double> m_average_points;

    FaceMorphFilter m_morph_filter;
    TextureFilter m_texture_filter;

    Framebuffer m_output_fb;
};

NAMESPACE_END