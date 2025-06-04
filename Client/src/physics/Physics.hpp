// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Physics
#include "Vec2.hpp"

// Global
#include "Timer.hpp"

// C++ standard libraries
#include <memory>

struct Intersect
{
    bool result = false;
    Vec2f pos; // position of intersection
};

namespace Physics
{
    /// @brief calculates overlap rectangle size of the bounding boxes of entity a and entity b
    /// @return a Vec2f of (x overlap, y overlap)
    // Vec2f OverlapAABB(Entity a, Entity b)
    Vec2f OverlapAABB(const Vec2f& aPos, const Vec2f& aBoxHalfSize, const Vec2f& bPos, const Vec2f& bBoxHalfSize)
    {
        PROFILE_FUNCTION();

        float xDiff = abs(aPos.x - bPos.x);
        float yDiff = abs(aPos.y - bPos.y);

        float xOverlap = aBoxHalfSize.x + bBoxHalfSize.x - xDiff;
        float yOverlap = aBoxHalfSize.y + bBoxHalfSize.y - yDiff;

        return Vec2f(xOverlap, yOverlap); // if xOverlap > 0, AABB overlap of xOverlap in the x-direction
    }

    /// @brief check if a pixel position pos in the game world is inside of entity e's bounding box (for axis-aligned only)
    bool IsInside(const Vec2f& pixelPos, const Vec2f& entityPos, const Vec2f& entityBoxHalfSize)
    {
        bool horizontalInside = pixelPos.x > entityPos.x - entityBoxHalfSize.x && pixelPos.x < entityPos.x + entityBoxHalfSize.x;
        bool verticalInside = pixelPos.y > entityPos.y - entityBoxHalfSize.y && pixelPos.y < entityPos.y + entityBoxHalfSize.y;

        return horizontalInside && verticalInside;
    }

    /// @brief determine if line AB intersects with line CD
    Intersect LineIntersect(const Vec2f& a, const Vec2f& b, const Vec2f& c, const Vec2f& d)
    {
        Vec2f r = b - a;
        Vec2f s = d - c;
        float rxs = r.cross(s);
        Vec2 cma = c - a;
        float t = (cma.cross(s)) / rxs;
        float u = (cma.cross(r)) / rxs;
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
        {
            return { true, Vec2f(a.x + t * r.x, a.y + t * r.y) };
        }
        else
        {
            return { false, Vec2f(0, 0) };
        }
    }

    /// TODO: implement this function
    // bool EntityIntersect(const Vec2f& a, const Vec2f& b /* entity e */)
    // {
    //     return false;
    // }

    /// @brief check for overlap among two convex polygons
    // bool OverlapSAT()

    /// @brief apply force at pos to rectangular entity and update entity's velocity and angular acceleration
    void ForceEntity(const Vec2f& entityPos, Vec2f& entityVel, float& entityAngVel, const Vec2f& entityBoxSize, const Vec2f& force, const Vec2f& forcePos)
    {
        /// TODO: expand to real physics with mass components and such if needed or easier or smoother

        Vec2f r = forcePos - entityPos; // center of mass (centroid here) of ragdoll to bullet location
        float torque = r.cross(force);
        /// TODO: make sure this is right and makes sense ^ std::cout << sqrtf(tangentialForce^2 + linearForce^2) == 1 << std::endl;
        float I = 1.0f / 12.0f * (entityBoxSize.x * entityBoxSize.x + entityBoxSize.y * entityBoxSize.y) * 2.0f; // moment of inertia for a rectangle /// TODO: should include factor of player mass too if doing real physics
        float alpha = torque / I; // angular acceleration, same as angular velocity if force is impulse (which it is)

        entityVel += force; // arbitrary choice, change in velocity (acceleration) should be F / mass
        entityAngVel += alpha;
    }
}
