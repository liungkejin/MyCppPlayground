//
// Created by LiangKeJin on 2024/7/28.
//

#pragma once

#include <iostream>

class TimeUtils {
public:
    static int64_t nowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch())
                .count();
    }

    static int64_t nowUs() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch())
                .count();
    }
};
