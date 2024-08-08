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

    std::string imageDir = std::string(ASSERTS_PATH) + "/images/raw";
    {
        std::string aImgPath = imageDir + "/2015:02.jpg";
        cv::Mat src = cv::imread(aImgPath);

        DetectResult &srcFP = faceDetector.detect(src.data, src.cols, src.rows,
                                                  HF_CAMERA_ROTATION_0, HF_STREAM_BGR, true);
        FaceData *f = srcFP.face(0);
        std::vector<float> points;
        if (f) {
            for (int k = 0; k < f->numLandmarks(); ++k) {
                points.push_back(f->landmarkX(k));
                points.push_back(f->landmarkY(k));
            }
        }

        faceMorph.setSrcImg(src.data, src.cols, src.rows, GL_BGR);
        Landmark landmark(src.cols, src.rows, points);
        faceMorph.setSrcKeyPoints(landmark, 55, 105, 22);
    }

    std::string bImgPath = imageDir + "/2000:01.jpg";
    cv::Mat dst = cv::imread(bImgPath);
    {
        DetectResult &srcFP = faceDetector.detect(dst.data, dst.cols, dst.rows,
                                                  HF_CAMERA_ROTATION_0, HF_STREAM_BGR, true);
        FaceData *f = srcFP.face(0);
        std::vector<float> points;
        if (f) {
            for (int k = 0; k < f->numLandmarks(); ++k) {
                points.push_back(f->landmarkX(k));
                points.push_back(f->landmarkY(k));
            }
        }
        faceMorph.setDstImg(dst.data, dst.cols, dst.rows, GL_BGR);
        Landmark landmark(dst.cols, dst.rows, points);
        faceMorph.setDstKeyPoints(landmark, 55, 105, 22);

        if (false) {
            cv::Mat img = dst;
            cv::Mat cl;
            float scale = 1920.f / img.cols;
            cv::resize(img, cl, cv::Size(img.cols * scale, img.rows * scale));
            cv::Rect rect(f->x() * scale, f->y() * scale, f->width() * scale, f->height() * scale);
            cv::rectangle(cl, rect, cv::Scalar(255, 255, 0), 2);

            // 55 105 22
            for (int k = 0; k < f->numLandmarks(); ++k) {
                cv::Mat ddd = cl.clone();

                if (k != 55 && k != 105) {
                    continue;
                }

                cv::Point2f p(f->landmarkX(k), f->landmarkY(k));
                printf("landmark（%d): %.2f, %.2f\n", k,  p.x, p.y);

                cv::Point2f drawP = p * scale;
                cv::circle(ddd, drawP, 0, cv::Scalar(0, 0, 255), 2);
                std::string index = std::to_string(k);
                cv::putText(ddd, index, drawP, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0));

                cv::imshow("11", ddd);
                cv::waitKey(0);
            }
        }
    }

//    faceMorph.generateTriangles();
}

TextureFilter textureFilter;
Texture2D texture2D = Texture2D(100, 75);

void GLFaceMorph::test(int width, int height, float percent) {
    if (!init_flag) {
        initialize();
        init_flag = true;
    }

    Framebuffer &fb = faceMorph.render(percent);

    textureFilter.setTextureCoord(0, false, true);
    textureFilter.inputTexture(fb.textureNonnull());
    textureFilter.setViewport(width, height);

    GLRect rect = GLRect::fitCenter(fb.texWidth(), fb.texHeight(), width, height);
    textureFilter.setVertexCoord(rect, width, height);
    textureFilter.render();
}

NAMESPACE_END