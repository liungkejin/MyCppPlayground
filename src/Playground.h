//
// Created by LiangKeJin on 2024/7/27.
//
#pragma once

#include "Log.h"

#define ASSERTS_PATH "../assets"

#define ERROR_CODE_NULLPTR (-12345)

// 为了防止缩进，IDE 无法修改namespace的缩进
#define NAMESPACE_BEG(ns) namespace ns {
#define NAMESPACE_WUTA NAMESPACE_BEG(wuta)
#define NAMESPACE_END }

#define _FATAL_IF(condition, fmt, args...)                                                                            \
    if (condition) {                                                                                                   \
        _FATAL(fmt, ##args);                                                                                           \
    }

#define _ERROR_RETURN_IF(condition, retcode, fmt, args...)                                                             \
    if (condition) {                                                                                                   \
        _ERROR(fmt, ##args);                                                                                           \
        return (retcode);                                                                                              \
    }

#define _ERROR_IF(condition, fmt, args...)                                                                             \
    if (condition) {                                                                                                   \
        _ERROR(fmt, ##args);                                                                                           \
    }

#define _WARN_RETURN_IF(condition, retcode, fmt, args...)                                                              \
    if (condition) {                                                                                                   \
        _WARN(fmt, ##args);                                                                                            \
        return (retcode);                                                                                              \
    }

#define _WARN_IF(condition, fmt, args...)                                                                              \
    if (condition) {                                                                                                   \
        _WARN(fmt, ##args);                                                                                            \
    }

#define _INFO_IF(condition, fmt, args...)                                                                              \
    if (condition) {                                                                                                   \
        _INFO(fmt, ##args);                                                                                            \
    }

#define DELETE_TO_NULL(ptr)  if (ptr) { delete ptr; ptr = nullptr; }

#define DELETE_ARR_TO_NULL(ptr)  if (ptr) { delete [] ptr; ptr = nullptr; }
