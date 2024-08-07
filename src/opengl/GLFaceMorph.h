//
// Created by LiangKeJin on 2024/7/28.
//

#pragma once

#include <Playground.h>
#include "wrap/filter/FaceMorphFilter.h"
#include "wrap/filter/TextureFilter.h"
#include "utils/Delaunator.h"
#include "face/CVUtils.h"

NAMESPACE_WUTA

struct TransStatus {
    float scale;
    float rotate;
    float trans_x;
    float trans_y;
};

class Landmark {
public:
    Landmark() {}

    Landmark(float w, float h, const std::vector<float> &p, bool glCoord = false) : m_width(w), m_height(h) {
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

    Landmark &scale(float sa) {
        for (int i = 0, size = (int) m_points.size(); i < size; ++i) {
            m_points[i] *= sa;
        }
        return *this;
    }

    Landmark &translate(float dx, float dy) {
        for (int i = 0, size = (int) m_points.size(); i < size; i += 2) {
            m_points[i] += dx;
            m_points[i + 1] += dy;
        }
        return *this;
    }

    Landmark &rotate(float cx, float cy, float angle) {
        for (int i = 0, size = (int) m_points.size(); i < size; i += 2) {
            float x = m_points[i];
            float y = m_points[i + 1];
            MathUtils::rotatePoint(x, y, cx, cy, angle);
            m_points[i] = x;
            m_points[i + 1] = y;
        }
        return *this;
    }

    Landmark &changeCoord(bool glCoord) {
        if (m_gl_coord == glCoord) {
            return *this;
        }
        m_gl_coord = glCoord;
        for (int i = 0, size = (int) m_points.size(); i < size; i += 2) {
            m_points[i + 1] = m_height - m_points[i + 1];
        }
        return *this;
    }

    std::vector<float> normalize() {
        std::vector<float> pv;
        for (int i = 0, size = (int) m_points.size(); i < size; i += 2) {
            pv.push_back(m_points[i] / m_width);
            pv.push_back(m_points[i + 1] / m_height);
        }
        return pv;
    }

    std::vector<float> trianglePoints() {
        std::vector<float> pv;
        float minx = (float) INT32_MAX, miny = (float) INT32_MAX, maxx = INT32_MIN, maxy = INT32_MIN;
        for (int i = 0, size = (int) m_points.size(); i < size; i += 2) {
            float x = m_points[i] / m_width;
            float y = m_points[i + 1] / m_height;
            if (minx > x) {
                minx = x;
            }
            if (maxx < x) {
                maxx = x;
            }
            if (miny > y) {
                miny = y;
            }
            if (maxy < y) {
                maxy = y;
            }

            pv.push_back(x);
            pv.push_back(y);
        }
        // 缩放矩形
        float cx = (maxx + minx) / 2.0f;
        float cy = (maxy + miny) / 2.0f;
        float lw = (maxx - minx) * 2.0f;
        float lh = (maxy - miny) * 2.0f;
        float lx = cx - lw / 2.0f, ly = cy - lh / 2.0f;
        float rx = cx + lw / 2.0f, ry = cy + lh / 2.0f;
        pv.push_back(lx);
        pv.push_back(ly);

        pv.push_back(rx);
        pv.push_back(ly);

        pv.push_back(rx);
        pv.push_back(ry);

        pv.push_back(lx);
        pv.push_back(ry);

        // 四个角
//        pv.push_back(MIN(0, lx));
//        pv.push_back(MIN(0, ly));
//
//        pv.push_back(MAX(1, rx));
//        pv.push_back(MAX(1, ry));
//
//        pv.push_back(MAX(1, rx));
//        pv.push_back(MIN(0, ly));
//
//        pv.push_back(MIN(0, lx));
//        pv.push_back(MAX(1, ry));
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
    void setKeyPoints(const Landmark &landmark, int leftEye, int rightEye, int nose) {
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

    inline const Landmark &landmark() {
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
        texFilter.setViewport(viewport).setVertexCoord(rect, (float) dw, (float) dh).setFullTextureCoord();
        texFilter.inputTexture(rawTexture()).render(&m_scaled_fb);

        m_width = dw;
        m_height = dh;
        m_landmark.setSize((float) dw, (float) dh);
    }

    Texture inputTexture() {
        return m_scaled_fb.valid() ? m_scaled_fb.textureNonnull() : rawTexture();
    }

    TransStatus getFinalTransStatus(MorphImage &dst) const {
        float sumScale = dst.eyeDistance() / eyeDistance();
        TransStatus status = {
                .scale = sumScale,
                .rotate = dst.eyeAngle() - eyeAngle(),
                .trans_x = dst.eyeCenterX() - eyeCenterX() * sumScale,
                .trans_y = dst.eyeCenterY() - eyeCenterY() * sumScale
        };
        return status;
    }

    /**
     * 得到一张渲染到指定位置的纹理，同时输出最终的点
     */
    Texture2D transform(TextureFilter &texFilter, const TransStatus &status, Landmark &outLandmark) {
        outLandmark = m_landmark;
        outLandmark.scale(status.scale).translate(status.trans_x, status.trans_y);
        float cx = outLandmark.centerX(m_leye_index, m_reye_index);
        float cy = outLandmark.centerY(m_leye_index, m_reye_index);
        outLandmark.rotate(cx, cy, status.rotate);

        GLRect rect((float) m_width, (float) m_height);
        rect.scale(status.scale, status.scale)
                .translate(status.trans_x, status.trans_y)
                .setRotation(cx, cy, status.rotate);

        m_trans_fb.create(m_width, m_height);
        texFilter.viewport().set(m_width, m_height)
                .enableClearColor(0, 0, 0, 0);
        texFilter.setVertexCoord(rect, (float) m_width, (float) m_height).setFullTextureCoord();
        texFilter.blend(false).inputTexture(inputTexture()).render(&m_trans_fb);

        return m_trans_fb.textureNonnull();
    }

    float *obtainTexPoints(int size) {
        return m_tex_points.obtain<float>(size);
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

    Array m_tex_points;
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

        TransStatus finalTrans = m_src_img.getFinalTransStatus(m_dst_img);
        TransStatus srcStatus = {
                .scale = 1 + (finalTrans.scale - 1) * percent,
                .rotate = finalTrans.rotate * percent,
                .trans_x = finalTrans.trans_x * percent,
                .trans_y = finalTrans.trans_y * percent
        };

//        _INFO("cur src scale: %.2f, cur src trans(%.2f, %.2f)", curSrcScale, curSrcTransX, curSrcTransY);
        Landmark curSrcLandmark;
        Texture2D srcTex = m_src_img.transform(m_texture_filter, srcStatus, curSrcLandmark);
        m_output_fb.create(dstWidth, dstHeight);
        if (percent < 0.00001f) {
            m_texture_filter.viewport().set(dstWidth, dstHeight).enableClearColor(0, 0, 0, 1);
            m_texture_filter.setFullTextureCoord().setFullVertexCoord();
            m_texture_filter.inputTexture(srcTex).alpha(1);
            m_texture_filter.render(&m_output_fb);
            return m_output_fb;
        }

        int leyeIndex = m_src_img.leftEyeIndex(), reyeIndex = m_src_img.rightEyeIndex();
        float curDstScale = curSrcLandmark.distance(leyeIndex, reyeIndex) / m_dst_img.eyeDistance();
        TransStatus dstStatus = {
                .scale = curDstScale,
                .rotate = finalTrans.rotate * (1 - percent),
                .trans_x = curSrcLandmark.centerX(leyeIndex, reyeIndex) - m_dst_img.eyeCenterX() * curDstScale,
                .trans_y = curSrcLandmark.centerY(leyeIndex, reyeIndex) - m_dst_img.eyeCenterY() * curDstScale
        };
        Landmark curDstLandmark;
        Texture2D dstTex = m_dst_img.transform(m_texture_filter, dstStatus, curDstLandmark);
        if (percent > 0.99999f) {
            m_texture_filter.viewport().set(dstWidth, dstHeight).enableClearColor(0, 0, 0, 1);
            m_texture_filter.setFullTextureCoord().setFullVertexCoord();
            m_texture_filter.inputTexture(dstTex).alpha(1);
            m_texture_filter.render(&m_output_fb);
            return m_output_fb;
        }


        // 计算三角形
        std::vector<float> srcPoints = curSrcLandmark.trianglePoints();
        std::vector<float> dstPoints = curDstLandmark.trianglePoints();

        std::vector<double> averagePoints;
        for (int i = 0, size = (int) srcPoints.size(); i < size; ++i) {
            averagePoints.push_back((dstPoints[i] + srcPoints[i]) / 2.);
        }
        delaunator::Delaunator dela(averagePoints);

        // 处理三角形，主动填充边界
        size_t trianglePointSize = dela.triangles.size();
        int orgItemSize = (int) trianglePointSize * 2;
        int itemSize = orgItemSize + 6 * 8;
        float *srcTriPs = m_src_img.obtainTexPoints(itemSize);
        float *dstTriPs = m_dst_img.obtainTexPoints(itemSize);

        for (size_t i = 0; i < trianglePointSize; ++i) {
            size_t ti = dela.triangles[i] * 2;
            float sx = srcPoints[ti], sy = 1 - srcPoints[ti + 1];
            float dx = dstPoints[ti], dy = 1 - dstPoints[ti + 1];

            srcTriPs[i * 2] = sx;
            srcTriPs[i * 2 + 1] = sy;
            dstTriPs[i * 2] = dx;
            dstTriPs[i * 2 + 1] = dy;
        }
        fixTriangles(srcTriPs, orgItemSize, 0, 1);
        fixTriangles(dstTriPs, orgItemSize, 0, 1);

        float *weight = m_vertex_points.obtain<float>(itemSize);
        for (size_t i = 0; i < itemSize; ++i) {
            weight[i] = ((1 - percent) * srcTriPs[i] + percent * dstTriPs[i]) * 2 - 1;
        }
        fixTriangles(weight, orgItemSize, -1, 1);

        m_morph_filter.viewport().set(dstWidth, dstHeight).enableClearColor(0, 0, 0, 1);
        m_morph_filter.setVertexCoord(weight, itemSize, GL_TRIANGLES, itemSize / 2);
        m_morph_filter.setSrcTexCoord(srcTriPs, itemSize);
        m_morph_filter.setDstTexCoord(dstTriPs, itemSize);
        m_morph_filter.setSrcImg(srcTex);
        m_morph_filter.setDstImg(dstTex);
        m_morph_filter.setAlpha(percent);

        m_morph_filter.render(&m_output_fb);

        return m_output_fb;
    }

private:
    static void fixTriangles(float *points, int size, int min, int max) {
        float minx = (float) INT32_MAX, miny = (float) INT32_MAX, maxx = INT32_MIN, maxy = INT32_MIN;
        for (int i = 0; i < size; i += 2) {
            float x = points[i];
            float y = points[i + 1];
            if (minx > x) {
                minx = x;
            }
            if (maxx < x) {
                maxx = x;
            }
            if (miny > y) {
                miny = y;
            }
            if (maxy < y) {
                maxy = y;
            }
        }
        float lx = MIN(min, minx);
        float ly = MIN(min, miny);
        float rx = MAX(max, maxx);
        float ry = MAX(max, maxy);
        // 加8个三角形
        points[size++] = lx;
        points[size++] = ly;
        points[size++] = rx;
        points[size++] = ly;
        points[size++] = minx;
        points[size++] = miny;

        points[size++] = minx;
        points[size++] = miny;
        points[size++] = rx;
        points[size++] = ly;
        points[size++] = maxx;
        points[size++] = miny;

        points[size++] = maxx;
        points[size++] = miny;
        points[size++] = rx;
        points[size++] = ly;
        points[size++] = rx;
        points[size++] = ry;

        points[size++] = maxx;
        points[size++] = miny;
        points[size++] = rx;
        points[size++] = ry;
        points[size++] = maxx;
        points[size++] = maxy;

        points[size++] = maxx;
        points[size++] = maxy;
        points[size++] = rx;
        points[size++] = ry;
        points[size++] = lx;
        points[size++] = ry;

        points[size++] = maxx;
        points[size++] = maxy;
        points[size++] = lx;
        points[size++] = ry;
        points[size++] = minx;
        points[size++] = maxy;

        points[size++] = minx;
        points[size++] = maxy;
        points[size++] = lx;
        points[size++] = ry;
        points[size++] = lx;
        points[size++] = ly;

        points[size++] = minx;
        points[size++] = maxy;
        points[size++] = lx;
        points[size++] = ly;
        points[size++] = minx;
        points[size++] = miny;
    }

private:
    MorphImage m_src_img;
    MorphImage m_dst_img;

    Array m_vertex_points;

    FaceMorphFilter m_morph_filter;
    TextureFilter m_texture_filter;

    Framebuffer m_output_fb;
};

NAMESPACE_END