// Copyright 2025, William MacDonald, All Rights Reserved.

// Core
#include "Components.hpp"
#include "Entity.hpp"

// World
#include "world/TileType.hpp"

CTransform::CTransform(const Vec2f& p) : pos(p), prevPos(p) { }
CTransform::CTransform(const Vec2f& p, float a) : pos(p), prevPos(p), angle(a), prevAngle(a) { }
CTransform::CTransform(const Vec2f& p, const Vec2f& v, const Vec2f& sc, float angle, float angVel)
    : pos(p), prevPos(p), scale(sc), velocity(v), angle(angle), prevAngle(angle), angularVelocity(angVel)
{ }

CColor::CColor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), light(0) { }

CType::CType(TileType type) : type(type) { }

CLifespan::CLifespan(int duration) : lifespan(duration) { }

CDamage::CDamage(int d) : damage(d) { }

CInvincibility::CInvincibility(int t) : timeRemaining(t) { }

CHealth::CHealth(int m) : max(m), current(m) { }
CHealth::CHealth(int m, int c) : max(m), current(c) { }

CBoundingBox::CBoundingBox(const Vec2i& s) : size(s), halfSize(s.x / 2, s.y / 2) { }
CBoundingBox::CBoundingBox(const Vec2i& s, bool m, bool v) : size(s), halfSize(s.x / 2.0f, s.y / 2.0f), blockMove(m), blockVision(v) { }

CAnimation::CAnimation(const Animation& animation, bool r) : animation(animation), repeat(r) { }

CGravity::CGravity(float g) : gravity(g) { }

CState::CState(State s) : state(s) { }

CFire::CFire(int fr, float minAcc, float maxAcc) : fireRate(fr), minAccuracy(minAcc), accuracy(maxAcc), maxAccuracy(maxAcc) { }

CJointRelation::CJointRelation(const Entity& e, float minA, float maxA) : entityID(e.getID()), minAngle(minA), maxAngle(maxA) { }

CJointInfo::CJointInfo(const std::array<float, 3>& positions) : initJointOffsets(positions) { }

CSkelAnim::CSkelAnim(const std::vector<SkelAnim>& skelAnims) : skelAnims(skelAnims) { }
