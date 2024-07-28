//
// Created by mac on 2024/7/19.
//

#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <utils/Delaunator.h>
#include "utils/TimeUtils.h"

class Landmarks {
public:
    void push(float v) {
        m_points.push_back(v);
    }

    void setup(std::vector<float> &points) {
        m_points = points;
    }

    // 加上边框上 8 个点
    void setup(float w, float h, std::vector<float> &points) {
        m_points.clear();
        if (points.empty()) {
            return;
        }

        // 尽可能减少点的个数
        for (float &f: points) {
            m_points.push_back(f);
        }
        m_points.push_back(0);
        m_points.push_back(0);

        m_points.push_back(w - 1);
        m_points.push_back(0);

        m_points.push_back(0);
        m_points.push_back(h - 1);

        m_points.push_back(w - 1);
        m_points.push_back(h - 1);

//        m_points.push_back((w - 1) / 2.f);
//        m_points.push_back(0);
//
//        m_points.push_back(0);
//        m_points.push_back((h - 1) / 2.f);
//
//        m_points.push_back((w - 1) / 2.f);
//        m_points.push_back(h - 1);
//
//        m_points.push_back((w - 1) / 2.f);
//        m_points.push_back((h - 1) / 2.f);
    }

    inline int vSize() const {
        return (int) m_points.size();
    }

    inline float v(int i) const {
        return m_points[i];
    }

    inline int pSize() const {
        return (int) m_points.size() / 2;
    }

    inline float px(int i) const {
        return m_points[2 * i];
    }

    inline float py(int i) const {
        return m_points[2 * i + 1];
    }

    void getTriangles(int ai, int bi, int ci,
                      cv::Rect &boundRect,
                      std::vector<cv::Point2f> &cropPoints) const {
        std::vector<cv::Point2f> srcPoints = {
                cv::Point2f(px(ai), py(ai)),
                cv::Point2f(px(bi), py(bi)),
                cv::Point2f(px(ci), py(ci)),
        };

        cv::Rect2f outRect = cv::boundingRect(srcPoints);
        boundRect = outRect;

        cropPoints.emplace_back(px(ai) - outRect.x, py(ai) - outRect.y);
        cropPoints.emplace_back(px(bi) - outRect.x, py(bi) - outRect.y);
        cropPoints.emplace_back(px(ci) - outRect.x, py(ci) - outRect.y);
    }

private:
    std::vector<float> m_points;
};

class MorphImage {
public:
    void setup(cv::Mat &src, std::vector<float> &points, bool addBoarderPoint=true) {
        img = src;
        if (addBoarderPoint) {
            landmarks.setup((float)img.cols, (float)img.rows, points);
        } else {
            landmarks.setup(points);
        }
    }

    inline bool noFace() const {
        return landmarks.vSize() == 0;
    }

    cv::Mat morphTriangles(std::vector<size_t> &triangles, Landmarks &dst, bool debug = false) const {
        cv::Mat out = img.clone();
//        mask_img = cv::Mat(img.rows, img.cols, CV_8UC1);
//        dst_img = cv::Mat(img.rows, img.cols, img.type());
        int triSize = (int) triangles.size() / 3;

        for (int i = 0; i < triSize; i++) {
            int ai = (int) triangles[i * 3];
            int bi = (int) triangles[i * 3 + 1];
            int ci = (int) triangles[i * 3 + 2];
//            printf("triangles index[%d,%d,%d]\n", ai, bi, ci);
//            long start = TimeUtils::nowMs();
            morphTriangle(ai, bi, ci, dst, out, debug);
//            long costMs = TimeUtils::nowMs() - start;
            //printf("iterator(%d) cost: %ld ms\n", i, costMs);
        }

        return out;
    }

    void morphTriangle(int ai, int bi, int ci, Landmarks &dst, cv::Mat &outMat, bool debug = false) const {
        // 原图的三角形点
        cv::Rect srcRect;
        // 以外接矩形为原点的做标点
        std::vector<cv::Point2f> srcCropPoints;
        srcCropPoints.reserve(3);
        landmarks.getTriangles(ai, bi, ci, srcRect, srcCropPoints);
//        printf("src bound rect: %d, %d - %d, %d\n", srcRect.x, srcRect.y, srcRect.width, srcRect.height);
//        for (auto &p: srcCropPoints) {
//            printf("src crop p[%.2f, %.2f]\n", p.x, p.y);
//        }

        // 目标图的三角形点
        cv::Rect dstRect;
        std::vector<cv::Point2f> dstCropPoints;
        dstCropPoints.reserve(3);
        dst.getTriangles(ai, bi, ci, dstRect, dstCropPoints);
//        printf("dst bound rect: %d, %d - %d, %d\n", dstRect.x, dstRect.y, dstRect.width, dstRect.height);
//        for (auto &p: dstCropPoints) {
//            printf("dst crop p[%.2f, %.2f]\n", p.x, p.y);
//        }

        /// 生成 mask 图
        std::vector<cv::Point> dstCropPointsInt;
        dstCropPointsInt.reserve(dstCropPoints.size());
        for (cv::Point2f p: dstCropPoints) {
            dstCropPointsInt.emplace_back((int) p.x, (int) p.y);
        }
        cv::Mat mask = cv::Mat::zeros(dstRect.height, dstRect.width, CV_32FC1);
        cv::fillConvexPoly(mask, dstCropPointsInt, cv::Scalar(1), 8, 0);
        if (debug) {
            cv::imshow("mask", mask);
            cv::waitKey();
        }

        cv::Mat srcCropImg = img(cv::Range(srcRect.y, srcRect.y + srcRect.height),
                                 cv::Range(srcRect.x, srcRect.x + srcRect.width));
        if (debug) {
            cv::imshow("srcCropImg", srcCropImg);
            cv::waitKey();
        }

        /// 生成映射矩阵
        cv::Mat trans = cv::getAffineTransform(srcCropPoints, dstCropPoints);
        cv::Mat dstCropImg;
        /// 仿射变换
        cv::warpAffine(srcCropImg, dstCropImg, trans,
                       cv::Size(dstRect.width, dstRect.height),
                       cv::INTER_LINEAR,cv::BORDER_REFLECT_101);
        if (debug) {
            cv::imshow("dstCropImg", dstCropImg);
            cv::waitKey();
        }

        // 将 dstCropImg 混合 mask blend 到 outMat
        cv::Mat roi = outMat(dstRect);
        cv::Mat vertMask = 1 - mask;
        cv::blendLinear(roi, dstCropImg, vertMask, mask, roi);

        if (debug) {
            cv::imshow("outMat", outMat);
            cv::waitKey();
        }
    }

public:
    Landmarks landmarks;
    cv::Mat img;
};

class FaceMorph {
public:
    void setup(cv::Mat &src, std::vector<float> srcFacePoints,
               cv::Mat &dst, std::vector<float> dstFacePoints) {
        if (src.cols != dst.cols || src.rows != dst.rows) {
            throw std::runtime_error("src size != dst size");
        }
        m_src_img.setup(src, srcFacePoints);
        m_dst_img.setup(dst, dstFacePoints);

        if (srcFacePoints.size() == dstFacePoints.size()) {
            std::vector<double> averagePoints;
            for (int i = 0, size = m_src_img.landmarks.vSize(); i < size; ++i) {
                averagePoints.push_back((m_src_img.landmarks.v(i) + m_dst_img.landmarks.v(i)) / 2.);
            }
            // 生成三角形
            delaunator::Delaunator dela(averagePoints);
            m_triangles_indexes = dela.triangles;
        } else {
            m_triangles_indexes.clear();
        }

        printf("FaceMorph triangles size: %d\n", (int)m_triangles_indexes.size()/3);
    }

    cv::Mat getFrameAt(int index, int sumFrames, bool debug = false) {
        float alpha = sumFrames < 1 ? 0 : (float) (index+1) / (float) (sumFrames);
        if (index <= 0) {
            return m_src_img.img;
        }

        if (index >= sumFrames-1) {
            return m_dst_img.img;
        }

        if (m_triangles_indexes.empty() || m_src_img.noFace() || m_dst_img.noFace()) {
            cv::Mat blendMat;
            cv::addWeighted(m_src_img.img, 1 - alpha,
                            m_dst_img.img, alpha, 0, blendMat);
            if (debug) {
                cv::imshow("blend", blendMat);
                cv::waitKey();
            }
            return blendMat;
        }

        Landmarks weightLandmarks;
        for (int i = 0, size = m_src_img.landmarks.vSize(); i < size; ++i) {
            float a = m_src_img.landmarks.v(i);
            float b = m_dst_img.landmarks.v(i);
            float n = (1 - alpha) * a + alpha * b;
            weightLandmarks.push(n);
        }

        if (debug) {
            cv::imshow("src", m_src_img.img);
            cv::waitKey();
        }

        long startMs = TimeUtils::nowMs();
        cv::Mat srcWrap = m_src_img.morphTriangles(m_triangles_indexes, weightLandmarks);
        if (debug) {
            cv::imshow("srcWrap", srcWrap);
            cv::waitKey();
        }
        long costMs = TimeUtils::nowMs() - startMs;
        printf("src morph cost: %ld ms\n", costMs);

        if (debug) {
            cv::imshow("dst", m_dst_img.img);
            cv::waitKey();
        }
        startMs = TimeUtils::nowMs();
        cv::Mat dstWrap = m_dst_img.morphTriangles(m_triangles_indexes, weightLandmarks);
        if (debug) {
            cv::imshow("dstWrap", dstWrap);
            cv::waitKey();
        }
        costMs = TimeUtils::nowMs() - startMs;
        printf("dst morph cost: %ld ms\n", costMs);

        startMs = TimeUtils::nowMs();
        cv::Mat blendMat;
        cv::addWeighted(srcWrap, 1 - alpha, dstWrap, alpha, 0, blendMat);
        if (debug) {
            cv::imshow("blend", blendMat);
            cv::waitKey();
        }
        costMs = TimeUtils::nowMs() - startMs;
        printf("blend cost: %ld ms\n", costMs);

        return blendMat;
    }

private:
    MorphImage m_src_img;

    MorphImage m_dst_img;

    // 三角形索引
    std::vector<std::size_t> m_triangles_indexes;
};

class FaceMorphTest {
public:
    static void test();
};