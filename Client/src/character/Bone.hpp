#pragma once

#include "physics/Vec2.hpp"

struct Bone
{
    Vec2f pos { }; // position of the bone in pixels (relative to parent Bone, endpoint, point of rotation)
    float angle = 0.0f; // angle of the bone in radians (relative to parent Bone)
};
