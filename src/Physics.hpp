#pragma once

#include "Entity.hpp"

namespace Physics
{
    Vec2f GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
    {
        // TODO: return overlap rectangle size of the bounding boxes of entity a and b
        return Vec2f(0, 0);
    }

    Vec2f GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
    {
        // TODO: return previous overlap rectangle size of the bounding boxes of entity a and b
        // previous overlap uses the entity's previous position, otherwise its the same code
        return Vec2f(0, 0);
    }
}