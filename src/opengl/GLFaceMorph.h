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
    void setData(const uint8_t *data, int width, int height, GLenum format) {
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
        m_leye_index = leftEye;
        m_reye_index = rightEye;
        m_nose_index = nose;
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

    inline int pointsSize() {
        return m_raw_points.size() / 2;
    }

    float eyeDistance() {
        float lx = px(m_leye_index), ly = py(m_leye_index);
        float rx = px(m_reye_index), ry = py(m_reye_index);
        return sqrt((lx - rx) * (lx - rx) + (ly - ry) * (ly - ry));
    }

    float eyeCenterX() {
        return (px(m_leye_index) + px(m_reye_index)) / 2.f;
    }

    float eyeCenterY() {
        return (py(m_leye_index) + py(m_reye_index)) / 2.f;
    }

    // 计算眼睛和 x 轴的夹角，顺时针，注意判断鼻子的方位
    float eyeAngle() {
        float lx = px(m_leye_index), ly = py(m_leye_index);
        float rx = px(m_reye_index), ry = py(m_reye_index);

        float dx = rx - lx;
        float dy = ry - ly;
        float angle = atan2(dy, dx);
        return angle;
    }

    /**
     * 将图片和点都统一到 dw, dh 大小
     */
    Texture prepare(TextureFilter &texFilter, int dw, int dh) {
        if (m_width == dw && m_height == dh) {
            return m_init_fb.valid()? m_init_fb.textureNonnull() : rawTexture();
        }

        int sw = m_width, sh = m_height;
        float scale = std::min((float) dw / sw, (float) dh / sh);
        float ssw = sw * scale, ssh = sh * scale;
        float dx = dw / 2.f - ssw / 2.f, dy = dh / 2.f - ssh / 2.f;

        transformPoints(scale, ssw, dx, dy);

        GLRect rect(0, 0, sw, sh);
        rect.scale(scale, scale);
        rect.translate(dx, dy);

        m_init_fb.create(dw, dh);
        Viewport viewport(dw, dh);
        viewport.enableClearColor(0, 0, 0, 0);
        texFilter.setViewport(viewport);
        texFilter.inputTexture(rawTexture());
        texFilter.setVertexCoord(rect, dw, dh);
        texFilter.render(&m_init_fb);

        m_width = dw;
        m_height = dh;
        return m_init_fb.textureNonnull();
    }

    /**
     * 得到一张渲染到指定位置的纹理，同时输出最终的点
     */
    Texture transform(TextureFilter &texFilter,
                      float scale, float rotate, float dx, float dy, std::vector<float>& finalPoints) {
        finalPoints.clear();
        for (int i = 0, size = pointsSize(); i < size; i += 1) {
            finalPoints.push_back(px(i) * scale + dx);
            finalPoints.push_back(py(i) * scale + dx);
        }
        GLRect rect(0, 0, m_width, m_height);
        rect.scale(scale, scale);
//        rect.rotate(rotate);
        rect.translate(dx, dy);

        m_trans_fb.create(m_width, m_height);
        Viewport viewport(m_width, m_height);
        viewport.enableClearColor(0, 0, 0, 0);
        texFilter.setViewport(viewport);
        texFilter.setVertexCoord(rect, m_width, m_height);
        texFilter.inputTexture(m_init_fb.textureNonnull());
        texFilter.render(&m_trans_fb);

        return m_init_fb.textureNonnull();
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
        GLRect rect(0, 0, m_width, m_height);
        rect.scale(scale, scale);
        rect.rotate(rotate);
        rect.translate(tranX, tranY);

        transformPoints(scale, rotate, tranX, tranY);


        float dstScale = 1.0f / sumScale + (1 - 1.0f / sumScale) * percent;
        float dstRotate = -sumRotate * (1 - percent);
        float dstTranX = sumTranX * (1 - percent);
        float dstTranY = sumTranY * (1 - percent);
        dst.transformPoints(dstScale, dstRotate, dstTranX, dstTranY);
    }

private:
    const Texture& rawTexture() {
        return m_texture.valid() ? m_texture : m_img.textureNonnull();
    }

    void transformPoints(float scale, float rotate, float dx, float dy) {
        for (int i = 0, size = m_raw_points.size(); i < size; i += 2) {
            m_raw_points[i] = m_raw_points[i] * scale + dx;
            m_raw_points[i + 1] = m_raw_points[i + 1] * scale + dy;
        }
    }


private:
    int m_width = 0;
    int m_height = 0;
    ImageTexture m_img;
    Texture m_texture = INVALID_TEXTURE;
    std::vector<float> m_raw_points;
    int m_leye_index = 0;
    int m_reye_index = 0;
    int m_nose_index = 0;

    Framebuffer m_init_fb;

    Framebuffer m_trans_fb;
};

class GLFaceMorph {
public:
    static void test(int width, int height, float percent);

public:
    void setSrcImg(const uint8_t *data, int width, int height, GLenum format) {
        m_src_img.setData(data, width, height, format);
    }

    void setSrcTexture(int id, int width, int height) {
        m_src_img.setTexture(id, width, height);
    }

    void setSrcKeyPoints(const std::vector<float> &points, int leftEye, int rightEye, int nose) {
        m_src_img.setKeyPoints(points, leftEye, rightEye, nose);
    }

    void setDstImg(const uint8_t *data, int width, int height, GLenum format) {
        m_dst_img.setData(data, width, height, format);
    }

    void setDstTexture(int id, int width, int height) {
        m_dst_img.setTexture(id, width, height);
    }

    void setDstKeyPoints(const std::vector<float> &points, int leftEye, int rightEye, int nose) {
        m_dst_img.setKeyPoints(points, leftEye, rightEye, nose);
    }


    /**
     * 最重要的是计算融合之后的 三角形
     * @param percent
     * @return
     */
    Framebuffer &render(float percent) {
        m_morph_filter.setAlpha(percent);

        const Texture &dstTex = m_dst_img.prepare(m_texture_filter, m_dst_img.width(), m_dst_img.height());
        const Texture &srcTex = m_src_img.prepare(m_texture_filter, m_dst_img.width(), m_dst_img.height());

        if (m_src_img.pointsSize() < 2 || m_src_img.pointsSize() != m_dst_img.pointsSize()) {
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
    MorphImage m_src_img;
    MorphImage m_dst_img;


    Array m_dst_triangle_points;

    int m_triangle_pcount = 0;

    Array m_percent_triangle_points;

    std::vector<double> m_average_points;

    FaceMorphFilter m_morph_filter;
    TextureFilter m_texture_filter;

    Framebuffer m_output_fb;
};

NAMESPACE_END