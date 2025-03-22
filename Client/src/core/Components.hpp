// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "physics/Vec2.hpp"
#include "Animation.hpp"
#include "world/TileType.hpp"
#include "State.hpp"
#include "utility/Globals.hpp"

#include <string>
#include <array>
#include <chrono>

class Entity;

class Component {
public:
    bool exists = false;
};

/// NOTE: no const qualifiers for any members since components will be reused when a new entity is created in place of an inactive but previously active one

/// TODO: consider dividing this into mult components for memory efficiency if needed
class CTransform : public Component {
public:
    Vec2f pos{ 0.0f, 0.0f }; // center of entity, pixels
    Vec2f prevPos{ 0.0f, 0.0f }; // pos last frame, pixels
    Vec2f scale{ 1.0f, 1.0f }; // can use to change direction entity is facing with a negative x
    Vec2f velocity{ 0.0f, 0.0f }; // pixels/s
    float angle = 0.0f; // rotation angle from x-axis, rad /// NOTE: CW rotation since +y is downward
    float prevAngle = 0.0f; // prev rotation angle from x-axis, rad
    float angularVelocity = 0.0f; // rad/s

    CTransform() = default;
    CTransform(const Vec2f& p);
    CTransform(const Vec2f& p, float a);
    CTransform(const Vec2f& p, const Vec2f& v, const Vec2f& sc, float angle, float angVel);
};

class CColor : public Component {
public:
    uint8_t r = 0, g = 0, b = 0;
    uint8_t light = 0;

    CColor() = default;
    CColor(uint8_t r, uint8_t g, uint8_t b);
};

class CType : public Component {
public:
    TileType type = NONE;

    CType() = default;
    CType(TileType type);
};

class CLifespan : public Component {
public:
    int lifespan = 0;

    CLifespan() = default;
    CLifespan(int duration);
};

class CDamage : public Component {
public:
    int damage = 0;

    CDamage() = default;
    CDamage(int d);
};

class CInvincibility : public Component {
public:
    int timeRemaining = 0;

    CInvincibility() = default;
    CInvincibility(int t);
};

class CHealth : public Component {
public:
    int max = 0;
    int current = 0;

    CHealth() = default;
    CHealth(int m);
    CHealth(int m, int c);
};

/// TODO: consider splitting into keyboard, mouse, controller, touch, etc.
class CInput : public Component {
public:
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool shoot = false;

    CInput() = default;
};

class CBoundingBox : public Component {
public:
    Vec2i size{ 0, 0 };
    Vec2f halfSize{ 0.0f, 0.0f };
    bool blockMove = false;
    bool blockVision = false;

    CBoundingBox() = default;
    CBoundingBox(const Vec2i& s);
    CBoundingBox(const Vec2i& s, bool m, bool v);
};

class CAnimation : public Component {
public:
    Animation animation;
    bool repeat = false; // looping animation

    CAnimation() = default;
    CAnimation(const Animation& animation, bool r);
};

class CGravity : public Component {
public:
    float gravity = 0.0f; /// TODO: may want to make it not const if I want to change gravity and stuff

    CGravity() = default;
    CGravity(float g);
};

// class CMass : public Component
// {
// public:
//     float mass = 0.0f; // kg

//     CMass() = default;
//     CMass(float m) : mass(m) {}
// };

/// TODO: change this to be more efficient using numbers instead of strings, maybe enum, something else
class CState : public Component {
public:
    State state = IDLE;

    CState() = default;
    CState(State s);
};

class CFire : public Component {
public:
    int fireRate; // bullets/s
    float minAccuracy, accuracy, maxAccuracy; // affects bullet spread, increased with time, decreases with shots fired, 1 is max (perfect line), 0 in min (completely random angle)
    std::chrono::steady_clock::time_point lastShotTime = std::chrono::high_resolution_clock::now();

    CFire() = default;
    CFire(int fr, float minAcc, float maxAcc);
};

class CJointRelation : public Component {
public:
    EntityID entityID; // cannot use pointer to entity, must then change the way add entity returns copies, and entity map stores copies; cannot use entity cuz of circular dependency; cannot use entity reference because cannot be default initialized
    float minAngle, maxAngle; // angles defined for when player dies facing right

    CJointRelation() = default;
    CJointRelation(const Entity& e, float minA, float maxA);
};

class CJointInfo : public Component {
public:
    std::array<float, 3> initJointOffsets; // y offset only, 4-element array of positions, match indices to determine which joint matches with which entities; 4 is minimum size since thighs and upper arms will have 1 joint with limb and 1 of 2 possible joints positions (left and right the same) with torso and head with have another

    CJointInfo() = default;
    CJointInfo(const std::array<float, 3>& positions);
};

// class CFollowPlayer : public Component
// {
// public:
//     Vec2f home = { 0, 0 };
//     float speed = 0;

//     CFollowPlayer() = default;
//     CFollowPlayer(Vec2f& p, float s);
// };

// class CPatrol : public Component
// {
// public:
//     std::vector<Vec2f> positions;
//     size_t currentPosition = 0;
//     float speed = 0;

//     CPatrol() = default;
//     CPatrol(const std::vector<Vec2f>& pos, const float s);
// };
