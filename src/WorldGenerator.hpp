#pragma once

#include <string>
#include <vector>

#include "FastNoiseLite.h"

/// define a Tile struct to store positions and types of blocks/tiles/whatever in the game grid
/// TODO: will want to change type to int and create a big map (not in code) of word to number
struct TileInfo
{
    int x, y; // x, y position on grid
    std::string type;
};

class WorldGenerator
{
    int m_worldWidth;
    int m_worldHeight;
    int m_seaLevel = 25;
    FastNoiseLite m_noise;

    /// chunk-based storage for rendering and everything, but will use vector of pairs instead for now
    // std::unordered_map<std::pair<int, int>, std::vector<std::vector<std::string>>> chunks;
    // accessed like chunks[{chunkX, chunkY}][localX][localY] = ...;
    std::vector<TileInfo> m_tilePositions;

    /// @brief lay out dirt and stone layer, filling m_tilePositions
    void firstLayer()
    {
        std::string tileType = "none";
        for (int y = 0; y < m_worldHeight; y++)
        {
            for (int x = 0; x < m_worldWidth; x++)
            {
                if (y <= m_worldHeight / 3)
                {
                    tileType = "dirt";
                }
                else
                {
                    tileType = "stone";
                }
                m_tilePositions.push_back({x, y, tileType});
            }
        }
    }

    /// @brief add some dirt in stone and some stone in dirt
    void blockPatches()
    {
        float patchScale = 0.05f;    // controls dirt/stone patch frequency
        float dirtThreshold = 0.4f;  // threshold for creating dirt vein
        float stoneThreshold = 0.2f; // threshold for creating stone patch
        for (int y = 0; y < m_worldHeight; y++)
        {
            for (int x = 0; x < m_worldWidth; x++)
            {
                float patchNoise = m_noise.GetNoise((float)x * patchScale, (float)y * patchScale);
                if (patchNoise > stoneThreshold && y <= m_worldHeight / 3)
                {
                    m_tilePositions[y * m_worldWidth + x].type = "stone";
                }
                else if (patchNoise > dirtThreshold && y > m_worldHeight / 3)
                {
                    m_tilePositions[y * m_worldWidth + x].type = "dirt";
                }
            }
        }
    }

    /// TODO:
    /// @brief add other things like bedrock veins
    void addBedrock()
    {

    }

    /// @brief add caves
    void addCaves()
    {
        float caveScale = 0.05f;    // controls cave frequency
        float caveThreshold = 0.4f; // threshold for creating caves
        for (int y = 0; y < m_worldHeight; y++)
        {
            for (int x = 0; x < m_worldWidth; x++)
            {
                float caveNoise = m_noise.GetNoise((float)x * caveScale, (float)y * caveScale);
                if (caveNoise > caveThreshold)
                {
                    m_tilePositions[y * m_worldWidth + x].type = "background";
                }
            }
        }
    }

    void createSkyline()
    {
        /// 1D noise for skyline along x-axis

        std::vector<int> terrainHeights(m_worldWidth);
        int terrainDelta = 20; // controls max deviation from sea level
        int seaLevel = 25;
        float noiseScale = 0.5f;
        for (int x = 0; x < m_worldWidth; x++)
        {
            float noiseVal = m_noise.GetNoise(static_cast<float>(x) * noiseScale, 0.0f); // [-1, 1]
            terrainHeights[x] = static_cast<int>(noiseVal * terrainDelta + seaLevel);  // [seaLevel - terrainDelta, seaLevel + terrainDelta]
        }


        /// remove tiles above terrain heights

        m_tilePositions.erase(
            std::remove_if(
                m_tilePositions.begin(),
                m_tilePositions.end(),
                [&terrainHeights](const TileInfo &info) // capture terrain heights by reference in the lambda func's capture list
                {
                    return info.y < terrainHeights[info.x];
                }),
            m_tilePositions.end());
    }

public:
    /// @brief constructs WorldGenerator object, defines noise type as Perlin
    /// @param width the width of the game world (in grid units)
    /// @param height the height of the game world (in grid units)
    WorldGenerator(int width, int height)
        : m_worldWidth(width), m_worldHeight(height)
    {
        m_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    }

    void generateWorld()
    {
        firstLayer();
        blockPatches();
        addBedrock();
        addCaves();
        createSkyline();
    }

    const std::vector<TileInfo> &getTilePositions() const
    {
        return m_tilePositions;
    }
};