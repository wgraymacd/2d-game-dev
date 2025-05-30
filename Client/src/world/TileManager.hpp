// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// World
#include "Tile.hpp"

// Utility
#include "utility/ClientGlobals.hpp"

// C++ standard libraries
#include <vector>

class TileManager
{
    const int m_worldMaxCellsX = Settings::worldMaxCellsX;
    const int m_worldMaxCellsY = Settings::worldMaxCellsY;

    std::vector<Tile> m_tiles { static_cast<size_t>(m_worldMaxCellsX * m_worldMaxCellsY) };

    /// TODO: also consider chunk-based (map of pairs to chunks and chunks are 1d flat arrays of tiles) or quadtree storage, both are more efficient memory usage for more sparse worlds, efficient neighbor access, dynamic world size, and can still check neighbors with the indices regardless

public:

    TileManager() = default;

    std::vector<Tile>& getTiles()
    {
        return m_tiles;
    }

    void addTile(const Tile& tile, int x, int y) /// TODO: look into adding tiles like emplace back instead of copying, if possible
    {
        m_tiles.data()[x * m_worldMaxCellsY + y] = tile;
    }
};
