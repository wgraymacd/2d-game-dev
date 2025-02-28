#pragma once

#include <array>

#include "TileType.hpp"
#include "Vec2.hpp"

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
        for (int x = 0; x < 80; ++x)
        {
            for (int y = 0; y < 20; ++y)
            {
                if (x == 0 || x == 79 || y == 0 || y == 19)
                {
                    hallway[x][y] = BRICK;
                }
                else
                {
                    hallway[x][y] = BRICKWALL;
                }
            }
        }

        // small room
        // for (int x = 0; x < 20; ++x)
        // {
        //     for (int y = 0; y < 20; ++y)
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
        // for (int x = 0; x < 10; ++x)
        // {
        //     for (int y = 0; y < 40; ++y)
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
        // for (int x = 0; x < 60; ++x)
        // {
        //     for (int y = 0; y < 30; ++y)
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
        // for (int x = 0; x < 30; ++x)
        // {
        //     for (int y = 0; y < 20; ++y)
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