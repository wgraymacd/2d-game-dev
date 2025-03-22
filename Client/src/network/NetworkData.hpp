#pragma once

#include "utility/Globals.hpp"
#include "physics/Vec2.hpp"

enum DataType : uint8_t {
    POSITION,
    VELOCITY,
    SPAWN
};

struct NetworkData {
    DataType dataType;
    EntityID netID; // sent and received to and from client
    union { // send and receive different data types with union
        Vec2i intVec;
        Vec2f floatVec;
    } data;
};
