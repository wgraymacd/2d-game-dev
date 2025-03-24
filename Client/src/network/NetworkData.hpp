#pragma once

#include "utility/Globals.hpp"
#include "physics/Vec2.hpp"

enum DataType : uint8_t {
    POSITION,
    VELOCITY,
    SPAWN,
    LOCAL_SPAWN
};

struct NetworkData {
    DataType dataType;
    EntityID localID;
    EntityID netID;
    Vec2f data;
};
