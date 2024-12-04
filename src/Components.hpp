#pragma once

#include "Animation.hpp"
#include "Vec2.hpp"

class Component 
{
public:
    bool exists = false;
};

class CTransform : public Component
{
public:
    Vec2f pos = { 0.0, 0.0 };
    Vec2f prevPos = { 0.0, 0.0 }; // pos last frame
    Vec2f scale = { 1.0, 1.0 };
    Vec2f velocity = { 0.0, 0.0 };
    float rotAngle = 0;

    CTransform() = default;
    CTransform(const Vec2f& p)
        : pos(p) { }
    CTransform(const Vec2f& p, const Vec2f& v, const Vec2f& sc, float a)
        : pos(p), prevPos(p), velocity(v), scale(sc), rotAngle(a) { }
};

class CLifespan : public Component 
{
public:
    int lifespan = 0;
    int frameCreated = 0;

    CLifespan() = default;
    CLifespan(int duration, int frame)
        : lifespan(duration), frameCreated(frame) { }
};

class CInput : public Component
{
public:
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool shoot = false;
    bool canShoot = true;
    bool canJump = false;

    CInput() = default;
};

class CBoundingBox : public Component
{
public:
    Vec2f size;
    Vec2f halfSize;
    CBoundingBox() = default;
    CBoundingBox(const Vec2f& s)
        : size(s), halfSize(s.x / 2, s.y/2) { }
};

class CAnimation : public Component
{
public:
    Animation animation;
    bool repeat = false;
    CAnimation() = default;
    CAnimation(const Animation& animation, bool r)
        : animation(animation), repeat(r) { }
};

class CGravity : public Component
{
public:
    float gravity = 0;
    CGravity() = default;
    CGravity(float g) : gravity(g) { }
};

// bookkeeping
class CState: public Component
{
public:
    std::string state = "none";
    CState() = default;
    CState(const std::string& s) : state(s) { }
};
