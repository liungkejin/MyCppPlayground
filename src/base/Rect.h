//
// Created on 2024/7/4.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".


#pragma once
#include "Playground.h"

NAMESPACE_WUTA

class RectF {
public:
    RectF() {}
    RectF(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}
    RectF(const RectF &other) : x(other.x), y(other.y), width(other.width), height(other.height) {}

    RectF &operator=(const RectF &other) {
        x = other.x;
        y = other.y;
        width = other.width;
        height = other.height;
        return *this;
    }
    
    bool operator==(const RectF &other) const {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    void set(float x, float y, float width, float height) {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
    }

    void set(const RectF &other) { set(other.x, other.y, other.width, other.height); }

    bool isEmpty() const { return width <= 0 || height <= 0; }

    float area() const { return width * height; }

    float left() const { return x; }

    float top() const { return y; }

    float right() const { return x + width; }

    float bottom() const { return y + height; }

    float centerX() const { return x + width / 2.0f; }

    float centerY() const { return y + height / 2.0f; }

public:
    float x = 0, y = 0, width = 0, height = 0;
};

NAMESPACE_END
