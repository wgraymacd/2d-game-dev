// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "Vec2.hpp"
#include "Globals.hpp"

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
