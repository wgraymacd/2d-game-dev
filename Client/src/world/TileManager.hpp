// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include <vector>

#include "Tile.hpp"
#include "utility/Globals.hpp"

class TileManager
{
    std::vector<Tile> m_tiles; // tiles[x * ySize + y] = tile at grid pos (x, y)
    /// TODO: also consider chunk-based (map of pairs to chunks and chunks are 1d flat arrays of tiles) or quadtree storage, both are more efficient memory usage for more sparse worlds, efficient neighbor access, dynamic world size, and can still check neighbors with the indices regrdless

public:

    TileManager()
    {
        m_tiles = std::vector<Tile>(GlobalSettings::worldMaxCellsX * GlobalSettings::worldMaxCellsY);
    }

    std::vector<Tile>& getTiles()
    {
        return m_tiles;
    }

    void addTile(const Tile& tile, const int x, const int y) /// TODO: look into adding tiles like emplace back instead of copying, if possible
    {
        m_tiles[x * GlobalSettings::worldMaxCellsY + y] = tile;
    }
};