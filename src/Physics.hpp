// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "Vec2.hpp"
#include "Entity.hpp"
#include "Components.hpp"

#include "Timer.hpp"

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
    Vec2f OverlapAABB(Entity a, Entity b)
    {
        PROFILE_FUNCTION();

        /// TODO: Ensure the entities' positions and bounding boxes are updated correctly before these functions are called to reflect the right collision state
        /// TODO: either change ctrans to cpos for tiles or do tile collisions without a bounding box at all (just tile matrix and position and global settings for tile size), could also decouple pos and velocity in general, check player and other entities (velocity only used with position, but is pos accessed by itself more than the two together? if so, decouple)


        const Vec2f& aPos = a.getComponent<CTransform>().pos;
        const Vec2f& bPos = b.getComponent<CTransform>().pos;
        float xDiff = abs(aPos.x - bPos.x);
        float yDiff = abs(aPos.y - bPos.y);

        const Vec2f& aHalfBounds = a.getComponent<CBoundingBox>().halfSize;
        const Vec2f& bHalfBounds = b.getComponent<CBoundingBox>().halfSize;
        float xOverlap = aHalfBounds.x + bHalfBounds.x - xDiff;
        float yOverlap = aHalfBounds.y + bHalfBounds.y - yDiff;

        return Vec2f(xOverlap, yOverlap); // if xOverlap > 0, AABB overlap of xOverlap in the x-direction
    }

    /// @brief calculate previous overlap rectangle size of the bounding boxes of entity a and b
    /// @return a Vec2f of (previous x overlap, previous y overlap)
    Vec2f PrevOverlapAABB(Entity a, Entity b)
    {
        PROFILE_FUNCTION();

        const Vec2f& aPrevPos = a.getComponent<CTransform>().prevPos;
        const Vec2f& bPrevPos = b.getComponent<CTransform>().prevPos;
        float xDiff = abs(aPrevPos.x - bPrevPos.x);
        float yDiff = abs(aPrevPos.y - bPrevPos.y);

        const Vec2f& aHalfBounds = a.getComponent<CBoundingBox>().halfSize;
        const Vec2f& bHalfBounds = b.getComponent<CBoundingBox>().halfSize;
        float xOverlap = aHalfBounds.x + bHalfBounds.x - xDiff;
        float yOverlap = aHalfBounds.y + bHalfBounds.y - yDiff;

        return Vec2f(xOverlap, yOverlap); // if xOverlap > 0, AABB overlap of xOverlap in the x-direction
    }



    /// @brief check if a pixel position pos in the game world is inside of entity e's bounding box (for axis-aligned only)
    bool IsInside(const Vec2f& pos, Entity e)
    {
        const Vec2f& entityPos = e.getComponent<CTransform>().pos;
        const Vec2f& bBoxHalfSize = e.getComponent<CBoundingBox>().halfSize;

        bool horizontalInside = pos.x > entityPos.x - bBoxHalfSize.x && pos.x < entityPos.x + bBoxHalfSize.x;
        bool verticalInside = pos.y > entityPos.y - bBoxHalfSize.y && pos.y < entityPos.y + bBoxHalfSize.y;

        return horizontalInside && verticalInside;
    }

    /// @brief determine if line AB intersects with line CD
    Intersect LineIntersect(const Vec2f& a, const Vec2f& b, const Vec2f& c, const Vec2f& d)
    {
        Vec2f r = b - a;
        Vec2f s = d - c;
        float rxs = r.cross(s);
        Vec2 cma = c - a;
        float t = (cma.cross(s)) / r.cross(s);
        float u = (cma.cross(r)) / r.cross(s);
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
    bool EntityIntersect(const Vec2f& a, const Vec2f& b, std::shared_ptr<Entity> e)
    {
        return false;
    }

    /// @brief check for overlap among two convex polygons
    // bool OverlapSAT()

    /// @brief apply force at pos to rectangular ecntity and update entity's velocity and angular acceleration
    void ForceEntity(const Entity& entity, const Vec2f& force, const Vec2f& pos)
    {
        CTransform& trans = entity.getComponent<CTransform>();
        CBoundingBox& box = entity.getComponent<CBoundingBox>();

        /// TODO: expand to real physics with mass components and such if needed or easier or smoother

        Vec2f r = pos - trans.pos; // center of mass (centroid here) of ragdoll to bullet location
        float torque = r.cross(force);
        /// TODO: make sure this is right and makes sense ^ std::cout << sqrtf(tangentialForce^2 + linearForce^2) == 1 << std::endl;
        float I = 1.0f / 12.0f * (box.size.x * box.size.x + box.size.y * box.size.y) * 2.0f; // moment of inertia for a rectangle /// TODO: should include factor of player mass too if doing real physics
        float alpha = torque / I; // angular acceleration, same as angular velocity if force is impulse (which it is)

        trans.velocity += force; // arbitrary choice, change in velocity (acceleration) should be F / mass
        trans.angularVelocity += alpha;
    }
}