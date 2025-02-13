#pragma once 

#include <SFML/Graphics.hpp>
#include <math.h>
#include <limits>

template <typename T> // if hardcoded to be floats, might want to use doubles later, just make a template
class Vec2
{
public:
    T x = 0;
    T y = 0;

    Vec2() = default; // explicitly tell compiler to use default constructor

    Vec2(T xin, T yin) : x(xin), y(yin) {}

    /// @brief constructor to convert from sf::Vector2
    /// @param vec a reference to an sf::Vector2 object
    Vec2(const sf::Vector2<T>& vec) : x(vec.x), y(vec.y) {}

    /// @brief automatic conversion to sf::Vector2, allows passing Vec2 objects to SFML functions
    operator sf::Vector2<T>() const
    {
        return sf::Vector2<T>(x, y);
    }

    /// @brief convert from Vec2<T> to Vec2<U>
    /// @tparam U any type (int, float, etc.)
    /// @return a Vec2<U> with the same values as the original Vec2<T> if a static cast is available from T to U
    template <typename U>
    Vec2<U> to() const
    {
        return Vec2<U>(static_cast<U>(x), static_cast<U>(y));
    }

    // use of const: const Vec2& rhs ensures that the original object rhs is not modified and const at the end guarantees that this func does not modify the object it is called on

    Vec2 operator + (const Vec2& rhs) const
    {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2 operator - (const Vec2& rhs) const
    {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator / (const T val) const
    {
        return Vec2(x / val, y / val);
    }

    Vec2 operator * (const T val) const
    {
        return Vec2(x * val, y * val);
    }

    void operator += (const Vec2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
    }

    void operator -= (const Vec2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
    }

    void operator /= (const T val)
    {
        x /= val;
        y /= val;
    }

    void operator *= (const T val)
    {
        x *= val;
        y *= val;
    }

    bool operator == (const Vec2& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }

    bool operator != (const Vec2& rhs) const
    {
        return (x != rhs.x) && (y != rhs.y);
    }

    /// TODO: make distSquared() since often don't need to sqrt to do comparisons
    /// @brief calculates the length of the vector from vec to the object this function is called on
    /// @param vec a Vec2 object you want to find the distance to
    /// @return the length of the connecting vector
    float dist(const Vec2& vec) const
    {
        return sqrtf((x - vec.x) * (x - vec.x) + (y - vec.y) * (y - vec.y));
    }

    /// @brief get the manhattan distance of this entity from the point vec
    /// @return a Vec2 of the x-distance and the y-distance
    Vec2 distManhattan(const Vec2& vec) const
    {
        return Vec2(abs(x - vec.x), abs(y - vec.y));
    }

    /// @brief take the cross product of this cross vec
    /// @return the new Vec2 cross product
    float cross(const Vec2& vec) const
    {
        return x * vec.y - vec.x * y;
    }

    /// TODO: change this, it's a lil confusing naming, maybe just use angle
    /// @brief return the angle of the vector formed by vec - this from the +x-axis, [-π, π]
    float angleFrom(const Vec2& vec) const
    {
        float angle = atan2f(y - vec.y, x - vec.x);
        return (angle >= 0) ? angle : (angle + 2 * M_PI);

        // return atan2f(y - vec.y, x - vec.x); // returns angles from -π to π, implicitly casts arguments to floats
    }

    /// @brief return the angle of the vector from the +x-axis, [-π, π]
    float angle() const
    {
        float angle = atan2f(y, x);
        return (angle >= 0) ? angle : (angle + 2 * M_PI);
    }

    /// @brief get the length of this vector
    float length() const
    {
        return sqrtf(x * x + y * y);
    }

    /// @brief get this vector's slope
    float slope() const
    {
        if (x == 0) return std::numeric_limits<float>::max();
        return static_cast<float>(y) / static_cast<float>(x);
    }

    /// @brief return this vector's unit vector
    /// @return 
    Vec2 norm() const
    {
        float length = this->length();
        if (length == 0) return Vec2(0, 0);
        return Vec2(x / length, y / length);
    }
};


/// shotcuts

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
// using Vec2i16 = Vec2<int16_t>;
// using Vec2ui = Vec2<unsigned int>;
// using Vec2ui8 = Vec2<uint8_t>;
// using Vec2ui16 = Vec2<uint16_t>;
// using Vec2ui32 = Vec2<uint32_t>;


/// hash function for Vec2i

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