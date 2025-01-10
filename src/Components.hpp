#pragma once

#include "Animation.hpp"
#include "Vec2.hpp"
#include <string>
#include <vector>

class Component
{
public:
    bool exists = false;
};

/// TODO: consider dividing this into mult components for memory efficiency if needed
class CTransform : public Component
{
public:
    Vec2f pos = { 0.0, 0.0 }; // top-left corner of entity
    Vec2f prevPos = { 0.0, 0.0 }; // pos last frame
    Vec2f scale = { 1.0, 1.0 }; // can use to change direction entity is facing with a negative x
    Vec2f velocity = { 0.0, 0.0 };
    float rotAngle = 0; // rotation angle from x-axis in radians

    CTransform() = default;
    CTransform(const Vec2f& p)
        : pos(p) {
    }
    CTransform(const Vec2f& p, const Vec2f& v, const Vec2f& sc, float a)
        : pos(p), prevPos(p), velocity(v), scale(sc), rotAngle(a) {
    }
};

class CLifespan : public Component
{
public:
    int lifespan = 0;
    int frameCreated = 0;

    CLifespan() = default;
    CLifespan(int duration, int frame)
        : lifespan(duration), frameCreated(frame) {
    }
};

class CDamage : public Component
{
public:
    int damage = 1;

    CDamage() = default;
    CDamage(int d)
        : damage(d) {
    }
};

class CInvincibility : public Component
{
public:
    int iframes = 0;

    CInvincibility() = default;
    CInvincibility(int f)
        : iframes(f) {
    }
};

class CHealth : public Component
{
public:
    int max = 1;
    int current = 1;

    CHealth() = default;
    CHealth(int m, int c)
        : max(m), current(c) {
    }
};

/// TODO: consider splitting into keyboard, mouse, controller, touch, etc.
class CInput : public Component
{
public:
    // movement
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool canJump = false;

    // shooting
    bool shoot = false;
    bool canShoot = true;

    CInput() = default;
};

class CBoundingBox : public Component
{
public:
    Vec2i size;
    Vec2f halfSize;
    bool blockMove = false;
    bool blockVision = false;

    CBoundingBox() = default;
    CBoundingBox(const Vec2i& s)
        : size(s), halfSize(s.x / 2, s.y / 2) {
    }
    CBoundingBox(const Vec2i& s, bool m, bool v)
        : size(s), blockMove(m), blockVision(v), halfSize(s.x / 2.0f, s.y / 2.0f) {
    }
};

class CAnimation : public Component
{
public:
    Animation animation;
    bool repeat = false;

    CAnimation() = default;
    CAnimation(const Animation& animation, bool r)
        : animation(animation), repeat(r) {
    }
};

class CGravity : public Component
{
public:
    float gravity = 0;

    CGravity() = default;
    CGravity(float g) : gravity(g) {}
};

/// TODO: change this to be more efficient using numbers instead of strings, maybe enum, something else
class CState : public Component
{
public:
    std::string state = "none"; // values: "stand", "run", "air"

    CState() = default;
    CState(const std::string& s) : state(s) {}
};

class CFollowPlayer : public Component
{
public:
    Vec2f home = { 0, 0 };
    float speed = 0;

    CFollowPlayer() = default;
    CFollowPlayer(Vec2f& p, float s)
        : home(p), speed(s) {
    }
};

class CPatrol : public Component
{
public:
    std::vector<Vec2f> positions;
    size_t currentPosition = 0;
    float speed = 0;

    CPatrol() = default;
    CPatrol(std::vector<Vec2f>& pos, float s)
        : positions(pos), speed(s) {
    }
};
