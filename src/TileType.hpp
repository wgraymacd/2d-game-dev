#pragma once

enum TileType {
    NONE = 0,
    DIRT = 1, // easy to destroy
    STONE = 2, // harder to destroy, bullets can ricochet
    GLASS = 3, // same health as rock but does not block vision, ricochet changes with angle 
    BEDROCK = 4 // cannot break
};