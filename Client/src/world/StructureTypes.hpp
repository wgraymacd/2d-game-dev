// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// World
#include "TileType.hpp"

// Physics
#include "physics/Vec2.hpp"

// C++ standard libraries
#include <array>


struct StructureTypes
{
    std::array<std::array<TileType, 20>, 80> hallway;
    // std::array<std::array<TileType, 20>, 20> smallRoom;
    // std::array<std::array<TileType, 40>, 10> elevatorShaft;
    // std::array<std::array<TileType, 30>, 60> grandHall;
    // std::array<std::array<TileType, 20>, 30> bunker;

    StructureTypes()
    {
        // hallway
        for (size_t x = 0; x < 80; ++x)
        {
            for (size_t y = 0; y < 20; ++y)
            {
                if (x == 0 || x == 79 || y == 0 || y == 19)
                {
                    hallway[x][y] = TileType::BRICK;
                }
                else
                {
                    hallway[x][y] = TileType::BRICKWALL;
                }
            }
        }

        // small room
        // for (size_t x = 0; x < 20; ++x)
        // {
        //     for (size_t y = 0; y < 20; ++y)
        //     {
        //         if (x == 0 || x == 19 || y == 0 || y == 19)
        //         {
        //             smallRoom[x][y] = BRICK;
        //         }
        //         else if (y == 19 && x == 10)
        //         {
        //             smallRoom[x][y] = DOOR;
        //         }
        //         else if ((x == 2 && y == 2) || (x == 17 && y == 2))
        //         {
        //             smallRoom[x][y] = LIGHT;
        //         }
        //         else
        //         {
        //             smallRoom[x][y] = WOOD;
        //         }
        //     }
        // }

        // elevator shaft
        // for (size_t x = 0; x < 10; ++x)
        // {
        //     for (size_t y = 0; y < 40; ++y)
        //     {
        //         if (x == 0 || x == 9)
        //         {
        //             elevatorShaft[x][y] = METAL;
        //         }
        //         else if (y % 10 == 0)
        //         {
        //             if (x == 4 || x == 5)
        //                 elevatorShaft[x][y] = DOOR;
        //             else
        //                 elevatorShaft[x][y] = METAL;
        //         }
        //         else
        //         {
        //             elevatorShaft[x][y] = AIR;
        //         }
        //     }
        // }

        // grand hall
        // for (size_t x = 0; x < 60; ++x)
        // {
        //     for (size_t y = 0; y < 30; ++y)
        //     {
        //         if (x == 0 || x == 59 || y == 0 || y == 29)
        //         {
        //             grandHall[x][y] = MARBLE;
        //         }
        //         else if ((x % 10 == 0) && (y > 2 && y < 27))
        //         {
        //             grandHall[x][y] = COLUMN;
        //         }
        //         else if (y == 2 && (x >= 25 && x <= 35))
        //         {
        //             grandHall[x][y] = LIGHT;
        //         }
        //         else
        //         {
        //             grandHall[x][y] = (x % 2 == y % 2) ? CHECKER_BLACK : CHECKER_WHITE;
        //         }
        //     }
        // }

        // bunker
        // for (size_t x = 0; x < 30; ++x)
        // {
        //     for (size_t y = 0; y < 20; ++y)
        //     {
        //         if (x == 0 || x == 29 || y == 0 || y == 19)
        //         {
        //             bunker[x][y] = METAL;
        //         }
        //         else if (y == 0 && x == 15)
        //         {
        //             bunker[x][y] = WEAK_DIRT;
        //         }
        //         else if ((x == 2 && y == 2) || (x == 27 && y == 2))
        //         {
        //             bunker[x][y] = LIGHT;
        //         }
        //         else
        //         {
        //             bunker[x][y] = STEEL_FLOOR;
        //         }
        //     }
        // }
    }
};
