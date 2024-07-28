//
// Created by LiangKeJin on 2024/7/28.
//

#include <sys/stat.h>
#include <Playground.h>
#include "FaceMorph.h"
#include "utils/TimeUtils.h"
#include "face/detect/InspireFaceDetector.h"


const char *DST_IMG_DIR = "output";

int FPS = 30;
float duration = 1.0f;
int FRAME_TOTAL_FRAMES = (int) (FPS * duration);

void saveToFile(cv::Mat &mat, int i, int subI) {
    char filePath[128] = {0};
    sprintf(filePath, "%s/frame_%d-%d.jpg", DST_IMG_DIR, i, subI);
    cv::imwrite(filePath, mat);
}

class FaceImage {
public:
    explicit FaceImage(const char *p) : path(p) {
        cv::Mat m = cv::imread(p);
        cv::resize(m, img, cv::Size(m.cols/2, m.rows/2));
    }

    void detect(InspireFaceDetector &detector, bool debug = false) {
        long start = TimeUtils::nowMs();
        DetectResult &result = detector.detect(img.data, img.cols, img.rows,
                                               HF_CAMERA_ROTATION_0, HF_STREAM_BGR, true);

        long costMs = TimeUtils::nowMs() - start;
        printf("detect(%s) cost : %ld ms\n", path.c_str(), costMs);
        printf("result: num faces: %d\n", result.numFaces());

        landmarks.clear();
        if (result.numFaces() == 0) {
            return;
        }

        FaceData *f = result.face(0);
        printf("face data: track id: %d, num landmarks: %d\n", f->trackId(), f->numLandmarks());

        if (debug) {
            cv::Mat cl;
            float scale = 1920.f / img.cols;
            cv::resize(img, cl, cv::Size(img.cols * scale, img.rows * scale));
            cv::Rect rect(f->x() * scale, f->y() * scale, f->width() * scale, f->height() * scale);
            cv::rectangle(cl, rect, cv::Scalar(255, 255, 0), 2);

            for (int k = 0; k < f->numLandmarks(); ++k) {
                cv::Point2f p(f->landmarkX(k), f->landmarkY(k));
                printf("landmark: %.2f, %.2f\n", p.x, p.y);

                cv::Point2f drawP = p * scale;
                cv::circle(cl, drawP, 0, cv::Scalar(0, 0, 255), 2);
                std::string index = std::to_string(k);
                cv::putText(cl, index, drawP, cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0, 255, 0));
            }
            cv::imshow("11", cl);
            cv::waitKey(0);
        }

        for (int k = 0; k < f->numLandmarks(); ++k) {
            cv::Point2f p(f->landmarkX(k), f->landmarkY(k));
            landmarks.push_back(p);
        }
    }

    std::vector<float> getMorphKeyPoints(bool full) {
        std::vector<float> points;
        if (landmarks.empty()) {
            return points;
        }
        if (full) {
            for (auto & p : landmarks) {
                points.push_back(p.x);
                points.push_back(p.y);
            }
        } else {
            points.push_back(landmarks[55].x);
            points.push_back(landmarks[55].y);
            points.push_back(landmarks[52].x);
            points.push_back(landmarks[52].y);
            points.push_back(landmarks[9].x);
            points.push_back(landmarks[9].y);
        }
        return points;
    }

    void morph(FaceImage &b, bool fullPoints=false) {
        faceMorph.setup(img, getMorphKeyPoints(fullPoints),
                        b.img, b.getMorphKeyPoints(fullPoints));
        for (int k = 0; k < FPS; ++k) {
            long startMs = TimeUtils::nowMs();
            cv::Mat mat = faceMorph.getFrameAt(k, FPS, false);
            long costMs = TimeUtils::nowMs() - startMs;
            printf("generate frame(%d),cost: %ld ms\n", k, costMs);

            cv::imshow("final", mat);
            cv::waitKey();

            saveToFile(mat, 0, k);
        }
    }

public:
    std::string path;
    FaceMorph faceMorph;

    cv::Mat img;
    std::vector<cv::Point2f> landmarks;
};

void FaceMorphTest::test() {
    mkdir(DST_IMG_DIR, 0755);
    std::string imageDir = std::string(ASSERTS_PATH) + "/images";
    std::string aImgPath = imageDir + "/2000:01.jpg";
    FaceImage aimage(aImgPath.c_str());
    std::string bImgPath = imageDir + "/2001:05.jpg";
    FaceImage bimage(bImgPath.c_str());

    DetectConfig detectConfig = {};

    long start = TimeUtils::nowMs();
    InspireFaceDetector faceDetector(detectConfig);
    long costMs = TimeUtils::nowMs() - start;
    printf("init face detector cost : %ld ms\n", costMs);

    aimage.detect(faceDetector);
    bimage.detect(faceDetector);

    aimage.morph(bimage, true);
}
