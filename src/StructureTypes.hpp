#pragma once

#include <array>

#include "TileType.hpp"
#include "Vec2.hpp"

struct StructureTypes
{
    std::array<std::array<TileType, 20>, 80> hallway;

    StructureTypes()
    {
        for (int x = 0; x < 80; ++x)
        {
            for (int y = 0; y < 20; ++y)
            {
                // draw collidable
                if (x == 0 || x == 79 || y == 0 || y == 19)
                {
                    hallway[x][y] = BRICK;
                }
                // draw background
                else
                {
                    hallway[x][y] = BRICKWALL;
                }
            }
        }
    }
};