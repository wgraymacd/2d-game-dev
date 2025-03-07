#pragma once

#include "utility/Globals.hpp"
#include "physics/Vec2.hpp"

enum DataType : uint8_t
{
    POSITION,
    VELOCITY,
    SPAWN
};

struct NetworkData
{
    DataType dataType;
    EntityID id;
    union // can only have one object from this list present at once, each Vec2 shared same memory location, largest member of union determines its size
    {
        EntityID netID;
        Vec2i intVec;
        Vec2f floatVec;
    } data;
};