#pragma once

#include "Vec2.hpp"
#include "Entity.hpp"
#include "Components.hpp"

#include "Timer.hpp"

#include <memory>

struct Intersect
{
    bool result = false;
    Vec2f pos;
};

namespace Physics
{
    /// @brief calculates overlap rectangle size of the bounding boxes of entity a and entity b
    /// @param a an entity in the current scene
    /// @param b an entity in the current scene
    /// @return a Vec2f of (x overlap, y overlap)
    Vec2f GetOverlap(Entity a, Entity b)
    {
        PROFILE_FUNCTION();

        /// TODO: Ensure the entities' positions and animations are updated correctly before these functions are called to reflect the right collision state


        const Vec2f& aPos = a.getComponent<CTransform>().pos;
        const Vec2f& bPos = b.getComponent<CTransform>().pos;
        float xDiff = abs(aPos.x - bPos.x);
        float yDiff = abs(aPos.y - bPos.y);

        const Vec2i& aAnimSize = a.getComponent<CAnimation>().animation.getSize();
        const Vec2i& bAnimSize = b.getComponent<CAnimation>().animation.getSize();
        float xOverlap = aAnimSize.x / 2 + bAnimSize.x / 2 - xDiff;
        float yOverlap = aAnimSize.y / 2 + bAnimSize.y / 2 - yDiff;

        return Vec2f(xOverlap, yOverlap); // if xOverlap > 0, AABB overlap of xOverlap in the x-direction
    }

    /// @brief calculate previous overlap rectangle size of the bounding boxes of entity a and b
    /// @param a an entity in the current scene
    /// @param b an entity in the current scene
    /// @return a Vec2f of (previous x overlap, previous y overlap)
    Vec2f GetPreviousOverlap(Entity a, Entity b)
    {
        PROFILE_FUNCTION();

        const Vec2f& aPrevPos = a.getComponent<CTransform>().prevPos;
        const Vec2f& bPrevPos = b.getComponent<CTransform>().prevPos;
        float xDiff = abs(aPrevPos.x - bPrevPos.x);
        float yDiff = abs(aPrevPos.y - bPrevPos.y);

        const Vec2i& aAnimSize = a.getComponent<CAnimation>().animation.getSize();
        const Vec2i& bAnimSize = b.getComponent<CAnimation>().animation.getSize();
        float xOverlap = aAnimSize.x / 2 + bAnimSize.x / 2 - xDiff;
        float yOverlap = aAnimSize.y / 2 + bAnimSize.y / 2 - yDiff;

        return Vec2f(xOverlap, yOverlap); // if xOverlap > 0, AABB overlap of xOverlap in the x-direction
    }

    /// TODO: implement this function
    bool IsInside(const Vec2f& pos, std::shared_ptr<Entity> e)
    {
        return false;
    }

    /// TODO: implement this function
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
}