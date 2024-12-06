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

    // constructor to convert from sf::Vector2
    Vec2(const sf::Vector2<T>& vec)
        : x(vec.x), y(vec.y)
    { }

    // allow automatic conversion to sf::Vector2
    // this allows passing Vec2 to sfml functions
    operator sf::Vector2<T>()
    {
        return sf::Vector2<T>(x, y);
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

    float dist(const Vec2& vec) const
    {
        return sqrtf((x - vec.x) * (x - vec.x) + (y - vec.y) * (y - vec.y));
    }
};

using Vec2f = Vec2<float>; // shortcut
using Vec2i = Vec2<int>; // shortcut