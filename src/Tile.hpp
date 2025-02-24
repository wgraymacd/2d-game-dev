#pragma once

#include <cstdint>
#include "TileType.hpp"

struct Tile
{
    uint8_t r, g, b, light;
    bool blocksVision, blocksMovement;
    int health; // if health is zero, tile inactive
    TileType type;
};