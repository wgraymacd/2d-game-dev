#pragma once

#include <cstdint>

enum TileType : uint8_t {
    NONE, // = 0
    DIRT, // easy to destroy
    STONE, // harder to destroy, bullets can ricochet
    GLASS, // same health as rock but does not block vision, ricochet changes with angle 
    BEDROCK // cannot break
};