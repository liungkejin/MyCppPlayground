//
// Created by LiangKeJin on 2024/7/28.
//

#pragma once

#include <Playground.h>
#include "wrap/filter/FaceMorphFilter.h"
#include "utils/Delaunator.h"

NAMESPACE_WUTA

class GLFaceMorph {
public:
    static void test(int width, int height, float percent);

public:
    void setSrcImg(const uint8_t * data, int width, int height, GLenum format=GL_RGBA) {
        m_src_img.set(data, width, height, format);
    }

    void setSrcFacePoints(const std::vector<float>& points, int width, int height, bool addBoard = true) {
        float * d = m_src_face_points.obtain<float>(points.size() + 12);
        int i = 0, size = (int)points.size();
        while (i <= size-2) {
            d[i] = points[i] / (float) width;
            d[i+1] = points[i+1] / (float) height;
            i += 2;
        }
        if (addBoard) {
            d[i++] = 0; d[i++] = 0;
            d[i++] = 1; d[i++] = 0;
            d[i++] = 0; d[i++] = 1;
            d[i++] = 1; d[i++] = 1;
        }
        m_src_pcount = i;
        _INFO("set src face points: %d", m_src_pcount);
    }

    void setDstImg(const uint8_t * data, int width, int height, GLenum format=GL_RGBA) {
        m_dst_img.set(data, width, height, format);
    }

    void setDstFacePoints(const std::vector<float>& points, int width, int height, bool addBoard = true) {
        float * d = m_dst_face_points.obtain<float>(points.size() + 12);
        int i = 0, size = (int)points.size();
        while (i <= size-2) {
            d[i] = points[i] / (float) width;
            d[i+1] = points[i+1] / (float) height;
            i += 2;
        }
        if (addBoard) {
            d[i++] = 0; d[i++] = 0;
            d[i++] = 1; d[i++] = 0;
            d[i++] = 0; d[i++] = 1;
            d[i++] = 1; d[i++] = 1;
        }
        m_dst_pcount = i;
        _INFO("set dst face points: %d", m_dst_pcount);
    }

    void generateTriangles() {
        if (m_src_pcount < 2 || m_src_pcount != m_dst_pcount) {
            m_triangle_pcount = 0;
            _WARN("no triangle was generated, src p: %d, dst p: %d", m_src_pcount, m_dst_pcount);
            return;
        }
        std::vector<double> averagePoints;
        for (int i = 0, size = m_src_pcount; i < size; ++i) {
            averagePoints.push_back((m_src_face_points.at<float>(i) + m_dst_face_points.at<float>(i)) / 2.);
        }

        // 生成三角形
        delaunator::Delaunator dela(averagePoints);

        m_triangle_pcount = (int)dela.triangles.size() * 2;
        float *src = m_src_triangle_points.obtain<float>(m_triangle_pcount);
        float *dst = m_dst_triangle_points.obtain<float>(m_triangle_pcount);
        int i = 0;
        for (size_t & t : dela.triangles) {
            src[i] = m_src_face_points.at<float>((int)t*2);
            src[i+1] = m_src_face_points.at<float>((int)t*2+1);
            dst[i] = m_dst_face_points.at<float>((int)t*2);
            dst[i+1] = m_dst_face_points.at<float>((int)t*2+1);
            i += 2;
        }
        _INFO("generate triangle success! triangles: %d, i: %d", m_triangle_pcount, i);
    }

    void setViewport(int width, int height) {
        m_morph_filter.setViewport(width, height);
    }

    void render(float percent, Framebuffer *framebuffer = nullptr) {
        m_morph_filter.setAlpha(percent);
        m_morph_filter.setSrcImg(m_src_img.textureNonnull());
        m_morph_filter.setDstImg(m_dst_img.textureNonnull());

        if (m_triangle_pcount < 3) {
            // 简单混合
            m_morph_filter.setVertexCoord(nullptr, 0);
            m_morph_filter.setSrcTexCoord(nullptr, 0);
            m_morph_filter.setDstTexCoord(nullptr, 0);
        } else {
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

        m_morph_filter.render(framebuffer);
    }

private:
    ImageTexture m_src_img;
    Array m_src_face_points;
    int m_src_pcount = 0;
    Array m_src_triangle_points;

    ImageTexture m_dst_img;
    Array m_dst_face_points;
    int m_dst_pcount = 0;
    Array m_dst_triangle_points;

    int m_triangle_pcount = 0;

    Array m_percent_triangle_points;

    FaceMorphFilter m_morph_filter;
};

NAMESPACE_END