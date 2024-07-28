//
// Created by LiangKeJin on 2024/7/28.
//

#include <opencv2/opencv.hpp>
#include "GLFaceMorph.h"
#include "face/detect/InspireFaceDetector.h"
#include "wrap/filter/TextureFilter.h"

NAMESPACE_WUTA

static bool init_flag = false;
static GLFaceMorph faceMorph;

void initialize() {
    DetectConfig detectConfig = {};
    InspireFaceDetector faceDetector(detectConfig);

    std::string imageDir = std::string(ASSERTS_PATH) + "/images";
    {
        std::string aImgPath = imageDir + "/2000:01.jpg";
        cv::Mat src = cv::imread(aImgPath);
        faceMorph.setSrcImg(src.data, src.cols, src.rows, GL_BGR);

        DetectResult &srcFP = faceDetector.detect(src.data, src.cols, src.rows,
                                                  HF_CAMERA_ROTATION_0, HF_STREAM_BGR, true);
        FaceData *f = srcFP.face(0);
        if (f) {
            std::vector<float> points;
            for (int k = 0; k < f->numLandmarks(); ++k) {
                points.push_back(f->landmarkX(k));
                points.push_back(f->landmarkY(k));
            }
            faceMorph.setSrcFacePoints(points, src.cols, src.rows);
        }
    }

    std::string bImgPath = imageDir + "/2001:05.jpg";
    cv::Mat dst = cv::imread(bImgPath);
    faceMorph.setDstImg(dst.data, dst.cols, dst.rows, GL_BGR);
    {
        DetectResult &srcFP = faceDetector.detect(dst.data, dst.cols, dst.rows,
                                                  HF_CAMERA_ROTATION_0, HF_STREAM_BGR, true);
        FaceData *f = srcFP.face(0);
        if (f) {
            std::vector<float> points;
            for (int k = 0; k < f->numLandmarks(); ++k) {
                points.push_back(f->landmarkX(k));
                points.push_back(f->landmarkY(k));
            }
            faceMorph.setDstFacePoints(points, dst.cols, dst.rows);
        }

//        if (true) {
//            cv::Mat img = dst;
//            cv::Mat cl;
//            float scale = 1920.f / img.cols;
//            cv::resize(img, cl, cv::Size(img.cols * scale, img.rows * scale));
//            cv::Rect rect(f->x() * scale, f->y() * scale, f->width() * scale, f->height() * scale);
//            cv::rectangle(cl, rect, cv::Scalar(255, 255, 0), 2);
//
//            for (int k = 0; k < f->numLandmarks(); ++k) {
//                cv::Point2f p(f->landmarkX(k), f->landmarkY(k));
//                printf("landmark: %.2f, %.2f\n", p.x, p.y);
//
//                cv::Point2f drawP = p * scale;
//                cv::circle(cl, drawP, 0, cv::Scalar(0, 0, 255), 2);
//                std::string index = std::to_string(k);
//                cv::putText(cl, index, drawP, cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0, 255, 0));
//            }
//            cv::imshow("11", cl);
//            cv::waitKey(0);
//        }
    }

    faceMorph.generateTriangles();
}

Framebuffer framebuffer;
TextureFilter textureFilter;

void GLFaceMorph::test(int width, int height, float percent) {
    if (!init_flag) {
        initialize();
        init_flag = true;
    }

    faceMorph.setViewport(width, height);
    framebuffer.create(width, height);
    faceMorph.render(percent, &framebuffer);

    textureFilter.setTextureCoord(0, false, true);
    textureFilter.inputTexture(framebuffer.textureNonnull());
    textureFilter.setViewport(width, height);
    textureFilter.render();
}

NAMESPACE_END