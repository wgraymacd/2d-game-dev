// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "Globals.hpp"

#include <cstdint>

enum DataType : uint8_t {
    POSITION,
    VELOCITY,
    SPAWN,
    LOCAL_SPAWN,
    WORLD_SEED,
    LOBBY_CONNECT
};

struct NetworkData {
    DataType dataType;

    EntityID localID;
    EntityID netID;

    float first;
    float second;
};
