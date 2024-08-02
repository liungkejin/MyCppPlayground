//
// Created by LiangKeJin on 2024/8/3.
//

#pragma once

#include <Playground.h>
#include <opencv2/opencv.hpp>

NAMESPACE_WUTA

class CVUtils {
public:
    static void displayFace(cv::Mat &img, const std::vector<float>& points) {

        cv::Mat drawImg = img.clone();
        // 55 105 22
        for (int k = 0, size = (int)points.size()/2; k < size; k += 1) {
            float x = points[k*2], y = points[k*2+1];

            cv::Point2f p(x, y);
            printf("landmark（%d): %.2f, %.2f\n", k,  p.x, p.y);

            cv::Point2f drawP(x, y);
            cv::circle(drawImg, drawP, 2, cv::Scalar(0, 0, 255), 2);
            std::string index = std::to_string(k);
            cv::putText(drawImg, index, drawP, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0));
        }

        cv::imshow("11", drawImg);
        cv::waitKey(0);
    }
};

NAMESPACE_END
