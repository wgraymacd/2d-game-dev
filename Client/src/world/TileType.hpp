// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// C++ standard libraries
#include <cstdint>
#include <ostream>

enum class TileType : uint8_t
{
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

/// @todo remove for release builds, or put this in its own code file or something (may be a bit too complex for inline)
inline std::ostream& operator<<(std::ostream& os, TileType type)
{
    switch (type)
    {
        case TileType::NONE: os << "NONE"; break;
        case TileType::DIRT: os << "DIRT"; break;
        case TileType::DIRTWALL: os << "DIRTWALL"; break;
        case TileType::STONE: os << "STONE"; break;
        case TileType::STONEWALL: os << "STONEWALL"; break;
        case TileType::BRICK: os << "BRICK"; break;
        case TileType::BRICKWALL: os << "BRICKWALL"; break;
        case TileType::GLASS: os << "GLASS"; break;
        case TileType::BULLETPROOFGLASS: os << "BULLETPROOFGLASS"; break;
        case TileType::BEDROCK: os << "BEDROCK"; break;
        default: os << "UNKNOWN"; break;
    }
    return os;
}
