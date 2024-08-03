//
// Created by LiangKeJin on 2024/7/28.
//

#pragma once

#include <Playground.h>
#include "wrap/filter/FaceMorphFilter.h"
#include "utils/Delaunator.h"
#include "opengl/wrap/filter/TextureFilter.h"
#include "face/CVUtils.h"

NAMESPACE_WUTA

class Landmark {
public:
    Landmark() {}

    Landmark(float w, float h, const std::vector<float> &p, bool glCoord=false) : m_width(w), m_height(h) {
        m_gl_coord = glCoord;
        m_points = p;
    }

    Landmark(const Landmark &l) {
        m_gl_coord = l.m_gl_coord;
        m_width = l.m_width;
        m_height = l.m_height;
        m_points = l.m_points;
    }

    Landmark &operator=(const Landmark &l) {
        m_gl_coord = l.m_gl_coord;
        m_width = l.m_width;
        m_height = l.m_height;
        m_points = l.m_points;
        return *this;
    }

    void setSize(float w, float h) {
        m_width = w;
        m_height = h;
    }

    inline float width() const {
        return m_width;
    }

    inline float height() const {
        return m_height;
    }

    inline int size() const {
        return (int) m_points.size() / 2;
    }

    inline float px(int index) const {
        return m_points[index * 2];
    }

    inline float py(int index) const {
        return m_points[index * 2 + 1];
    }

    float distance(int ai, int bi) const {
        float lx = px(ai);
        float ly = py(ai);
        float rx = px(bi);
        float ry = py(bi);
        return sqrt((lx - rx) * (lx - rx) + (ly - ry) * (ly - ry));
    }

    float centerX(int ai, int bi) const {
        return (px(ai) + px(bi)) / 2.f;
    }

    float centerY(int ai, int bi) const {
        return (py(ai) + py(bi)) / 2.f;
    }

    // 计算眼睛和 x 轴的夹角，顺时针
    float angle(int ai, int bi) const {
        float lx = px(ai);
        float ly = py(ai);
        float rx = px(bi);
        float ry = py(bi);

        float dx = rx - lx;
        float dy = ry - ly;
        return atan2(dy, dx);
    }

    Landmark& scale(float sa) {
        for (int i = 0, size = (int) m_points.size(); i < size; ++i) {
            m_points[i] *= sa;
        }
        m_width *= sa;
        m_height *= sa;
        return *this;
    }

    Landmark& translate(float dx, float dy) {
        for (int i = 0, size = (int) m_points.size(); i < size; i+=2) {
            m_points[i] += dx;
            m_points[i+1] += dy;
        }
        return *this;
    }

    Landmark& rotate(float cx, float cy, float angle) {
        for (int i = 0, size = (int) m_points.size(); i < size; i+=2) {
            float x = m_points[i];
            float y = m_points[i+1];
            MathUtils::rotatePoint(x, y, cx, cy, angle);
            m_points[i] = x;
            m_points[i+1] = y;
        }
        return *this;
    }

    Landmark& changeCoord(bool glCoord) {
        if (m_gl_coord == glCoord) {
            return *this;
        }
        m_gl_coord = glCoord;
        for (int i = 0, size = (int) m_points.size(); i < size; i+=2) {
            m_points[i+1] = m_height - m_points[i+1];
        }
        return *this;
    }

    std::vector<double> normalize(float w, float h) {
//        _INFO("size(%.2f, %.2f) , w: %.2f, H: %.2f", m_width, m_height, w, h);
        std::vector<double> pv;
        for (int i = 0, size = (int) m_points.size(); i < size; i+=2) {
            pv.push_back(m_points[i]/w);
            pv.push_back(m_points[i+1]/h);
        }
        return pv;
    }

private:
    bool m_gl_coord = false;

    float m_width = 0;
    float m_height = 0;
    std::vector<float> m_points;
};

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
    void setKeyPoints(const Landmark& landmark, int leftEye, int rightEye, int nose) {
        m_landmark = landmark;
        m_landmark.changeCoord(true);
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

    inline int pointsSize() const {
        return m_landmark.size();
    }

    inline bool canTransform(const MorphImage &dst) const {
        return pointsSize() > 1 && pointsSize() == dst.pointsSize();
    }

    inline const Landmark& landmark() {
        return m_landmark;
    }

    inline int leftEyeIndex() const { return m_leye_index; }

    inline int rightEyeIndex() const { return m_reye_index; }

    float eyeDistance() const {
        return m_landmark.distance(m_leye_index, m_reye_index);
    }

    float eyeCenterX() const {
        return m_landmark.centerX(m_leye_index, m_reye_index);
    }

    float eyeCenterY() const {
        return m_landmark.centerY(m_leye_index, m_reye_index);
    }

    // 计算眼睛和 x 轴的夹角，顺时针，注意判断鼻子的方位
    float eyeAngle() const {
        return m_landmark.angle(m_leye_index, m_reye_index);
    }

    /**
     * 将图片和点都统一到 dw, dh 大小
     */
    void scaleTo(TextureFilter &texFilter, int dw, int dh) {
        if (m_width == dw && m_height == dh) {
            return;
        }

        float sw = (float) m_width, sh = (float) m_height;
        float scale = std::min((float) dw / sw, (float) dh / sh);
        float ssw = sw * scale, ssh = sh * scale;
        float dx = (float) dw / 2.f - ssw / 2.f, dy = (float) dh / 2.f - ssh / 2.f;

        m_landmark.scale(scale).translate(dx, dy);

        GLRect rect(sw, sh);
        rect.scale(scale, scale);
        rect.translate(dx, dy);

        m_scaled_fb.create(dw, dh);
        Viewport viewport(dw, dh);
        viewport.enableClearColor(0, 0, 0, 0);
        texFilter.setViewport(viewport).setVertexCoord(rect, (float)dw, (float)dh).setFullTextureCoord();
        texFilter.inputTexture(rawTexture()).render(&m_scaled_fb);

        m_width = dw;
        m_height = dh;
        m_landmark.setSize(dw, dh);
    }

    Texture inputTexture() {
        return m_scaled_fb.valid() ? m_scaled_fb.textureNonnull() : rawTexture();
    }

    /**
     * 得到一张渲染到指定位置的纹理，同时输出最终的点
     */
    Texture2D transform(TextureFilter &texFilter,
                   float scale, float rotAngle, float dx, float dy, Landmark& outLandmark) {
        outLandmark = m_landmark;
        outLandmark.scale(scale).translate(dx, dy);
        float cx = outLandmark.centerX(m_leye_index, m_reye_index);
        float cy = outLandmark.centerY(m_leye_index, m_reye_index);
        outLandmark.rotate(cx, cy, rotAngle);

        GLRect rect((float) m_width, (float) m_height);
        rect.scale(scale, scale).translate(dx, dy).setRotation(cx, cy, rotAngle);

        m_trans_fb.create(m_width, m_height);
        Viewport viewport(m_width, m_height);
        viewport.enableClearColor(0, 0, 0, 0);
        texFilter.setViewport(viewport).setVertexCoord(rect, (float)m_width, (float)m_height).setFullTextureCoord();
        texFilter.blend(false).inputTexture(inputTexture()).render(&m_trans_fb);

        return m_trans_fb.textureNonnull();
    }

private:

    const Texture &rawTexture() {
        return m_texture.valid() ? m_texture : m_img.textureNonnull();
    }

private:
    int m_width = 0;
    int m_height = 0;
    ImageTexture m_img;
    Texture m_texture = INVALID_TEXTURE;

    Landmark m_landmark;
    int m_leye_index = 0;
    int m_reye_index = 0;
    int m_nose_index = 0;

    Framebuffer m_scaled_fb;

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

    void setSrcKeyPoints(const Landmark &landmark, int leftEye, int rightEye, int nose) {
        m_src_img.setKeyPoints(landmark, leftEye, rightEye, nose);
    }

    void setDstImg(const uint8_t *data, int width, int height, GLenum format) {
        m_dst_img.setData(data, width, height, format);
    }

    void setDstTexture(int id, int width, int height) {
        m_dst_img.setTexture(id, width, height);
    }

    void setDstKeyPoints(const Landmark &landmark, int leftEye, int rightEye, int nose) {
        m_dst_img.setKeyPoints(landmark, leftEye, rightEye, nose);
    }


    /**
     * 最重要的是计算融合之后的 三角形
     * @param percent
     * @return
     */
    Framebuffer &render(float percent) {

        int dstWidth = (int) m_dst_img.width();
        int dstHeight = (int) m_dst_img.height();
        m_src_img.scaleTo(m_texture_filter, dstWidth, dstHeight);

        if (!m_src_img.canTransform(m_dst_img)) {
            // 简单混合
            m_morph_filter.setFullVertexCoord();
            m_morph_filter.setSrcTexCoord(nullptr, 0);
            m_morph_filter.setDstTexCoord(nullptr, 0);

            m_output_fb.create(dstWidth, dstHeight);
            m_morph_filter.setViewport(dstWidth, dstHeight);
            m_morph_filter.render(&m_output_fb);
            return m_output_fb;
        }


        float sumScale = m_dst_img.eyeDistance() / m_src_img.eyeDistance();
        float sumRotate = m_dst_img.eyeAngle() - m_src_img.eyeAngle();
        float sumTranX = m_dst_img.eyeCenterX() - m_src_img.eyeCenterX() * sumScale;
        float sumTranY = m_dst_img.eyeCenterY() - m_src_img.eyeCenterY() * sumScale;

//        _INFO("sumScale: %.2f, sumRotate: %.2f, sumTranX: %.2f, sumTranY: %.2f", sumScale, sumRotate, sumTranX, sumTranY);
        float curSrcScale = 1 + (sumScale - 1) * percent;
        float curSrcRotate = sumRotate * percent;
        float curSrcTransX = sumTranX * percent;
        float curSrcTransY = sumTranY * percent;
//        _INFO("cur src scale: %.2f, cur src trans(%.2f, %.2f)", curSrcScale, curSrcTransX, curSrcTransY);
        Landmark curSrcLandmark;
        Texture2D srcTex = m_src_img.transform(m_texture_filter,
                                            curSrcScale, curSrcRotate,
                                            curSrcTransX, curSrcTransY, curSrcLandmark);

        float curDstScale = curSrcLandmark.distance(m_src_img.leftEyeIndex(), m_src_img.rightEyeIndex()) / m_dst_img.eyeDistance();
        float curDstRotate = sumRotate * (1 - percent);
        float curDstTransX = curSrcLandmark.centerX(m_src_img.leftEyeIndex(), m_src_img.rightEyeIndex()) - m_dst_img.eyeCenterX() * curDstScale;
        float curDstTransY = curSrcLandmark.centerY(m_src_img.leftEyeIndex(), m_src_img.rightEyeIndex()) - m_dst_img.eyeCenterY() * curDstScale;

        Landmark curDstLandmark;
        Texture2D dstTex = m_dst_img.transform(m_texture_filter,
                                             curDstScale, curDstRotate, curDstTransX, curDstTransY, curDstLandmark);

        m_output_fb.create(dstWidth, dstHeight);

//        Viewport vp(dstWidth, dstHeight);
//        vp.enableClearColor(0, 0, 0, 1);
//        m_texture_filter.setFullVertexCoord().setFullTextureCoord().setViewport(vp);
//        m_texture_filter.blend(false).inputTexture(srcTex).alpha(1).render(&m_output_fb);
////
//        m_texture_filter.setViewport(vp.disableClearColor());
//        m_texture_filter.blend(true).alpha(percent).inputTexture(dstTex).render(&m_output_fb);

        std::vector<double> srcNorm = curSrcLandmark.normalize(dstWidth, dstHeight);
        srcNorm.push_back(0);
        srcNorm.push_back(0);
        srcNorm.push_back(1);
        srcNorm.push_back(0);
        srcNorm.push_back(0);
        srcNorm.push_back(1);
        srcNorm.push_back(1);
        srcNorm.push_back(1);

        std::vector<double> dstNorm = curDstLandmark.normalize(dstWidth, dstHeight);
        dstNorm.push_back(0);
        dstNorm.push_back(0);
        dstNorm.push_back(1);
        dstNorm.push_back(0);
        dstNorm.push_back(0);
        dstNorm.push_back(1);
        dstNorm.push_back(1);
        dstNorm.push_back(1);

        std::vector<double> averagePoints;
        for (int i = 0, size = dstNorm.size(); i < size; ++i) {
            averagePoints.push_back((srcNorm[i] + dstNorm[i]) / 2.);
        }

        // 生成三角形
        delaunator::Delaunator dela(averagePoints);

//        _INFO("triangle size: %d", dela.triangles.size());
        int trianglePCount = (int) dela.triangles.size() * 2;
        float *src = m_src_triangle_points.obtain<float>(trianglePCount);
        float *dst = m_dst_triangle_points.obtain<float>(trianglePCount);
        int i = 0;
        for (size_t &t: dela.triangles) {
            src[i] = srcNorm[t * 2];
            src[i + 1] = 1 - srcNorm[t * 2 + 1];
            dst[i] = dstNorm[t * 2];
            dst[i + 1] = 1 - dstNorm[t * 2 + 1];
            i += 2;
        }

        float *weight = m_percent_triangle_points.obtain<float>(trianglePCount);
        for (i = 0; i < trianglePCount; ++i) {
            float a = src[i];
            float b = dst[i];
            weight[i] = ((1 - percent) * a + percent * b) * 2 - 1;
        }

//        m_output_fb.create(dstWidth, dstHeight);
//        Viewport vp(dstWidth, dstHeight);
//        vp.enableClearColor(0, 0, 0, 1);
//        m_texture_filter.setVertexCoord(weight, trianglePCount, GL_TRIANGLES, trianglePCount/2)
//                        .setTextureCoord(src, trianglePCount, GL_TRIANGLES, trianglePCount/2).setViewport(vp);
//        m_texture_filter.blend(false).inputTexture(srcTex).alpha(1).render(&m_output_fb);

        m_morph_filter.setViewport(dstWidth, dstHeight);
        m_morph_filter.setVertexCoord(weight, trianglePCount, GL_TRIANGLES, trianglePCount/2);
        m_morph_filter.setSrcTexCoord(src, trianglePCount);
        m_morph_filter.setDstTexCoord(dst, trianglePCount);
        m_morph_filter.setSrcImg(srcTex);
        m_morph_filter.setDstImg(dstTex);
        m_morph_filter.setAlpha(percent);


        m_output_fb.create(dstWidth, dstHeight);
        m_morph_filter.render(&m_output_fb);

        return m_output_fb;
    }

    void release() {
        // TODO
    }

private:
//    void generateTriangles(float width, float height, float percent) {
//        {
//            for (int i = 0, size = m_src_trans_points.size(); i < size; i += 2) {
//                m_src_trans_points[i] = m_src_trans_points[i] / width;
//                m_src_trans_points[i + 1] = m_src_trans_points[i + 1] / height;
//            }
//            m_src_trans_points.push_back(0);
//            m_src_trans_points.push_back(0);
//            m_src_trans_points.push_back(1);
//            m_src_trans_points.push_back(0);
//            m_src_trans_points.push_back(0);
//            m_src_trans_points.push_back(1);
//            m_src_trans_points.push_back(1);
//            m_src_trans_points.push_back(1);
//        }
//
//        {
//            for (int i = 0, size = m_dst_trans_points.size(); i < size; i += 2) {
//                m_dst_trans_points[i] = m_dst_trans_points[i] / width;
//                m_dst_trans_points[i + 1] = m_dst_trans_points[i + 1] / height;
//            }
//            m_dst_trans_points.push_back(0);
//            m_dst_trans_points.push_back(0);
//            m_dst_trans_points.push_back(1);
//            m_dst_trans_points.push_back(0);
//            m_dst_trans_points.push_back(0);
//            m_dst_trans_points.push_back(1);
//            m_dst_trans_points.push_back(1);
//            m_dst_trans_points.push_back(1);
//        }
//
//        m_average_points.clear();
//        for (int i = 0, size = m_src_trans_points.size(); i < size; ++i) {
//            m_average_points.push_back((m_src_trans_points[i] + m_dst_trans_points[i]) / 2.);
//        }
//
////        _INFO("generate triangle success! triangles: %d, i: %d", m_triangle_pcount, i);
//    }

private:
    MorphImage m_src_img;

    MorphImage m_dst_img;


    Array m_src_triangle_points;
    Array m_dst_triangle_points;

    Array m_percent_triangle_points;

    FaceMorphFilter m_morph_filter;
    TextureFilter m_texture_filter;

    Framebuffer m_output_fb;
};

NAMESPACE_END