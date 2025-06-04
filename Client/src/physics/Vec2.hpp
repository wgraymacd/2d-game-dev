// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// C++ standard libraries
#include <math.h>
#include <limits>
#include <functional>
#include <ostream>

template <typename T>
class Vec2
{
public:

    T x = 0;
    T y = 0;

    /// @brief default constructor for easy member initialization of classes that use Vec2
    constexpr Vec2() = default;

    constexpr Vec2(T xin, T yin) : x(xin), y(yin) { }

    /// @brief constructor to convert from sf::Vector2
    /// @param vec a reference to an sf::Vector2 object
    // Vec2(const sf::Vector2<T>& vec) : x(vec.x), y(vec.y) {}

    /// @brief automatic conversion to sf::Vector2, allows passing Vec2 objects to SFML functions
    // operator sf::Vector2<T>() const
    // {
    //     return sf::Vector2<T>(x, y);
    // }

    /// @brief convert from Vec2<T> to Vec2<U>
    template <typename U>
    constexpr Vec2<U> to() const
    {
        return Vec2<U>(static_cast<U>(x), static_cast<U>(y));
    }

    constexpr Vec2 operator-() const
    {
        return Vec2(-x, -y);
    }
    constexpr Vec2 operator+(const Vec2& rhs) const
    {
        return Vec2(x + rhs.x, y + rhs.y);
    }
    constexpr Vec2 operator-(const Vec2& rhs) const
    {
        return Vec2(x - rhs.x, y - rhs.y);
    }
    constexpr Vec2 operator/(const T val) const
    {
        return Vec2(x / val, y / val);
    }
    constexpr Vec2 operator*(const T val) const
    {
        return Vec2(x * val, y * val);
    }
    constexpr void operator+=(const Vec2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
    }
    constexpr void operator-=(const Vec2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
    }
    constexpr void operator/=(const T val)
    {
        x /= val;
        y /= val;
    }
    constexpr void operator*=(const T val)
    {
        x *= val;
        y *= val;
    }
    constexpr bool operator==(const Vec2& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }
    constexpr bool operator!=(const Vec2& rhs) const
    {
        return (x != rhs.x) || (y != rhs.y);
    }

    /// @brief returns the manhattan distance of this entity from the point vec
    Vec2 distManhattan(const Vec2& vec) const
    {
        return Vec2(abs(x - vec.x), abs(y - vec.y));
    }

    /// @brief returns the cross product of this cross vec
    float cross(const Vec2& vec) const
    {
        return x * vec.y - vec.x * y;
    }

    /// @brief returns the dot product of this vector with vec
    float dot(const Vec2& vec) const
    {
        return x * vec.x + y * vec.y;
    }

    /// @brief returns the angle of this vector from the positive x-axis, [-π, π]
    float angle() const
    {
        return atan2f(y, x);
    }

    /// @brief returns the length of this vector
    float length() const
    {
        return sqrtf(x * x + y * y);
    }

    /// @brief returns this vector's slope
    float slope() const
    {
        if (x == 0) return std::numeric_limits<float>::max();
        return static_cast<float>(y) / static_cast<float>(x);
    }

    /// @brief returns this vector's unit vector
    Vec2 norm() const
    {
        float length = this->length();
        if (length == 0) return Vec2(0, 0);
        return Vec2(x / length, y / length);
    }

    /// @brief returns a rotated version of this vector by angle, where angle is given from [-π, π]
    Vec2 rotate(const float angle) const
    {
        return Vec2(cosf(angle) * x - sinf(angle) * y, sinf(angle) * x + cosf(angle) * y);
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& out, const Vec2<T>& vec)
{
    out << "(" << vec.x << ", " << vec.y << ")";
    return out; // return std::ostream so we can chain calls to operator<<
}

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
// using Vec2ui = Vec2<unsigned int>;
// using Vec2uz = Vec2<size_t>;

/// @brief add hash functions for Vec2i and Vec2f to allow them to be used as keys in std::unordered_map
namespace std
{
    template <>
    struct hash<Vec2i>
    {
        std::size_t operator()(const Vec2i& v) const
        {
            return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
        }
    };

    template <>
    struct hash<Vec2f>
    {
        std::size_t operator()(const Vec2f& v) const
        {
            return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1);
        }
    };
}
