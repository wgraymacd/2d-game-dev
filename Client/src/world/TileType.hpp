// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include <cstdint>

enum TileType : uint8_t {
    NONE, // = 0
    DIRT, // easy to destroy
    DIRTWALL,
    STONE, // harder to destroy, bullets can ricochet
    STONEWALL,
    BRICK,
    BRICKWALL,
    GLASS, // little health but does not block vision, ricochet chance changes with angle 
    BULLETPROOFGLASS, // immune to bullets but not explosives for e.g., does not block vision, same ricochet as normal glass
    BEDROCK // cannot break at all
};
