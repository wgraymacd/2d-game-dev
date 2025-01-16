#pragma once

#include <string>
#include <vector>

#include "FastNoiseLite.h"

/// define a Tile struct to store positions and types of blocks/tiles/whatever in the game grid
/// TODO: will want to change type to int and create a big map (not in code) of word to number
// struct TileInfo
// {
//     int x, y; // x, y position on grid
//     std::string type;
// };

/// TODO: add biomes with specific rules for generation, could even define temp, humidity, etc. and calc tree density or water or weather or anything from them
/// TODO: enum for tile types
/// TODO: could add world evolution if I want people to be on same map for long time
class WorldGenerator
{
    int m_worldTilesX;
    int m_worldTilesY;
    int m_seaLevel = 25;

    /// TODO: change from string to int for tile types
    std::vector<std::vector<std::string>> m_tileMatrix;

    FastNoiseLite m_noise;

    /// TODO: try chunk-based storage for rendering and everything, but will use vector of pairs instead for now
    // std::unordered_map<std::pair<int, int>, std::vector<std::vector<std::string>>> chunks;
    // accessed like chunks[{chunkX, chunkY}][localX][localY] = ...;

    /// @brief lay out dirt and stone layer, filling m_tilePositions
    void generateBaseLayer()
    {
        std::string tileType = "";
        for (int y = 0; y < m_worldTilesY; y++)
        {
            for (int x = 0; x < m_worldTilesX; x++)
            {
                if (y <= m_worldTilesY / 3)
                {
                    tileType = "dirt";
                }
                else
                {
                    tileType = "stone";
                }
                m_tileMatrix[x][y] = tileType; /// NOTE: this is not stored as you would expect with (row, column), instead it's (x, y)
            }
        }
    }

    /// @brief add some dirt in stone and some stone in dirt
    void createBlockPatches()
    {
        // std::cout << "creating block patches" <<std::endl;
        float patchScale = 0.5f;    // controls dirt/stone patch frequency
        float dirtThreshold = 0.4f;  // threshold for creating dirt vein
        float stoneThreshold = 0.2f; // threshold for creating stone patch
        for (int y = 0; y < m_worldTilesY; y++)
        {
            for (int x = 0; x < m_worldTilesX; x++)
            {
                float patchNoise = m_noise.GetNoise((float)x * patchScale, (float)y * patchScale);
                if (patchNoise > stoneThreshold && y <= m_worldTilesY / 3)
                {
                    m_tileMatrix[x][y] = "stone";
                }
                else if (patchNoise > dirtThreshold && y > m_worldTilesY / 3)
                {
                    m_tileMatrix[x][y] = "dirt";
                }
            }
        }
    }

    /// TODO:
    /// @brief add other things like bedrock veins
    void addBedrock()
    {

    }

    /// TODO: Adding more sophisticated cave generation techniques, like cellular automata or Voronoi diagrams, could make your cave systems more organic and interesting
    /// @brief add caves
    void addCaves()
    {
        float caveScale = 0.5f;    // controls cave frequency
        float caveThreshold = 0.3f; // threshold for creating caves
        for (int y = 0; y < m_worldTilesY; y++)
        {
            for (int x = 0; x < m_worldTilesX; x++)
            {
                float caveNoise = m_noise.GetNoise((float)x * caveScale, (float)y * caveScale);
                if (caveNoise > caveThreshold)
                {
                    // m_tilePositions[y * m_worldTilesX + x].type = "background";
                    m_tileMatrix[x][y] = "background";
                }
            }
        }
    }

    /// TODO: could improve it by incorporating some horizontal variation and adding more diversity in terms of terrain features above the sea level
    void createSkyline()
    {
        // 1D noise for skyline along x-axis
        std::vector<int> terrainHeights(m_worldTilesX);
        int terrainDelta = 20; // controls max deviation from sea level
        int seaLevel = 25;
        float noiseScale = 0.5f;
        for (int x = 0; x < m_worldTilesX; x++)
        {
            float noiseVal = m_noise.GetNoise(static_cast<float>(x) * noiseScale, 0.0f); // [-1, 1]
            terrainHeights[x] = static_cast<int>(noiseVal * terrainDelta + seaLevel);  // [seaLevel - terrainDelta, seaLevel + terrainDelta]
        }

        // remove tiles above terrain heights
        for (int x = 0; x < m_worldTilesX; ++x)
        {
            for (int y = 0; y < m_worldTilesY; ++y)
            {
                if (y < terrainHeights[x])
                {
                    m_tileMatrix[x][y] = "";
                }
            }
        }
    }

public:
    /// @brief constructs WorldGenerator object, defines noise type as Perlin
    /// @param width the width of the game world (in grid units)
    /// @param height the height of the game world (in grid units)
    WorldGenerator(int numTilesX, int numTilesY)
        : m_worldTilesX(numTilesX), m_worldTilesY(numTilesY)
    {
        m_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        m_tileMatrix = std::vector<std::vector<std::string>>(m_worldTilesX, std::vector<std::string>(m_worldTilesY));
    }

    void generateWorld()
    {
        generateBaseLayer();
        createBlockPatches();
        addBedrock();
        addCaves();
        createSkyline();
    }

    std::vector<std::vector<std::string>> getTileMatrix()
    {
        return m_tileMatrix;
    }
};