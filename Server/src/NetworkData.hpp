// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "Vec2.hpp"
#include "Globals.hpp"

enum DataType : uint8_t
{
    POSITION,
    VELOCITY,
    SPAWN
};

struct NetworkData
{
    DataType dataType;
    NetEntityID id; // is SPAWN, this is a local entity ID, else it's a net ID
    union
    {
        NetEntityID netId; // if SPAWN, this is sent to client
        Vec2i intVec;
        Vec2f floatVec;
    } data;
};