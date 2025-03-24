// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once 

#include <math.h>
#include <limits>
#include <functional>

template <typename T>
class Vec2 {
public:
    T x;
    T y;

    Vec2() = default;

    Vec2(T xin, T yin) : x(xin), y(yin) {}

    /// @brief convert from Vec2<T> to Vec2<U>
    template <typename U>
    Vec2<U> to() const {
        return Vec2<U>(static_cast<U>(x), static_cast<U>(y));
    }

    Vec2 operator-() const {
        return Vec2(-x, -y);
    }

    Vec2 operator+(const Vec2& rhs) const {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2 operator-(const Vec2& rhs) const {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator/(const T val) const {
        return Vec2(x / val, y / val);
    }

    Vec2 operator*(const T val) const {
        return Vec2(x * val, y * val);
    }

    void operator+=(const Vec2& rhs) {
        x += rhs.x;
        y += rhs.y;
    }

    void operator-=(const Vec2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
    }

    void operator/= (const T val) {
        x /= val;
        y /= val;
    }

    void operator*=(const T val) {
        x *= val;
        y *= val;
    }

    bool operator==(const Vec2& rhs) const {
        return (x == rhs.x) && (y == rhs.y);
    }

    bool operator!=(const Vec2& rhs) const {
        return (x != rhs.x) && (y != rhs.y);
    }

    /// @brief get the manhattan distance of this entity from the point vec
    Vec2 distManhattan(const Vec2& vec) const {
        return Vec2(abs(x - vec.x), abs(y - vec.y));
    }

    /// @brief take the cross product of this cross vec
    float cross(const Vec2& vec) const {
        return x * vec.y - vec.x * y;
    }

    float dot(const Vec2& vec) const {
        return x * vec.x + y * vec.y;
    }

    /// @brief return the angle of the vector from the +x-axis, [-π, π]
    float angle() const {
        return atan2f(y, x); // implicitly casts arguments to floats
    }

    /// TODO: return length squared for faster calculations if only comparing lengths
    /// @brief get the length of this vector
    float length() const {
        return sqrtf(x * x + y * y);
    }

    /// @brief get this vector's slope
    float slope() const {
        if (x == 0) return std::numeric_limits<float>::max();
        return static_cast<float>(y) / static_cast<float>(x);
    }

    /// @brief return this vector's unit vector (use a Vec2f)
    Vec2 norm() const {
        float length = this->length();
        if (length == 0) return Vec2(0, 0);
        return Vec2(x / length, y / length);
    }

    /// @brief return a rotated version of this vector by angle, where angle is given from [-π, π]
    Vec2 rotate(const float angle) {
        return Vec2(cosf(angle) * x - sinf(angle) * y, sinf(angle) * x + cosf(angle) * y);
    }
};

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
