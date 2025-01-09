#pragma once 

#include <SFML/Graphics.hpp>
#include <math.h>

template <typename T> // if hardcoded to be floats, might want to use doubles later, just make a template
class Vec2
{
public:
    T x = 0;
    T y = 0;

    Vec2() = default; // explicitly tell compiler to use default constructor

    Vec2(T xin, T yin)
        : x(xin), y(yin)
    { }

    /// @brief constructor to convert from sf::Vector2
    /// @param vec a reference to an sf::Vector2 object
    Vec2(const sf::Vector2<T>& vec)
        : x(vec.x), y(vec.y)
    { }

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

    /// TODO: add a norm method so I don't have to do it in other code, maybe dot and cross prod as well
};

/// shotcuts

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;