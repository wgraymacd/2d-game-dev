// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include <cstdint>
#include "TileType.hpp"

/// TODO: consider finding way to eleminate redundancies between TileType and properties that are constant for that tile type like vision and movement blocks, max health
struct Tile
{
    TileType type = NONE;
    uint8_t r, g, b, light = 0;
    bool blocksVision, blocksMovement = false;
    int health = 0; // if health is zero, tile inactive
};