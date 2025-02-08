#pragma once

#include <string>
#include <vector>
#include <cstdint> // best practice for fixed bit width integers on all platforms like int16_t or uint32_t

#include "FastNoiseLite.h"
#include "TileType.hpp"

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
/// NOTE: if I use a certain seed, shit never changes, so can always get back to the same world
class WorldGenerator
{
    int m_worldTilesX;
    int m_worldTilesY;

    /// TODO: change from string to int for tile types
    std::vector<std::vector<TileType>> m_tileMatrix;

    FastNoiseLite m_noise;

    /// TODO: try chunk-based storage for rendering and everything, but will use vector of pairs instead for now
    // std::unordered_map<std::pair<int, int>, std::vector<std::vector<std::string>>> chunks;
    // accessed like chunks[{chunkX, chunkY}][localX][localY] = ...;

    /// @brief lay out dirt and stone layer, filling m_tilePositions
    void generateBaseLayer()
    {
        PROFILE_FUNCTION();

        std::cout << "creating base layer..." << std::endl;

        for (int y = 0; y < m_worldTilesY; ++y)
        {
            for (int x = 0; x < m_worldTilesX; ++x)
            {
                if (y <= m_worldTilesY / 3)
                {
                    m_tileMatrix[x][y] = DIRT;
                }
                else
                {
                    m_tileMatrix[x][y] = STONE;
                }
            }
        }
    }

    /// @brief add some dirt in stone and some stone in dirt
    void createBlockPatches()
    {
        PROFILE_FUNCTION();

        std::cout << "creating block patches..." << std::endl;

        float patchScale = 0.5f;    // controls dirt/stone patch frequency
        float dirtThreshold = 0.4f;  // threshold for creating dirt vein
        float stoneThreshold = 0.2f; // threshold for creating stone patch
        for (int y = 0; y < m_worldTilesY; ++y)
        {
            for (int x = 0; x < m_worldTilesX; ++x)
            {
                float patchNoise = m_noise.GetNoise(static_cast<float>(x) * patchScale, static_cast<float>(y) * patchScale); // returns val in range [-1, 1]
                if (patchNoise > stoneThreshold && y <= m_worldTilesY / 3)
                {
                    m_tileMatrix[x][y] = STONE;
                }
                else if (patchNoise > dirtThreshold && y > m_worldTilesY / 3)
                {
                    m_tileMatrix[x][y] = DIRT;
                }
            }
        }
    }

    /// TODO:
    /// @brief add other things like bedrock veins
    void addBedrock()
    {
        PROFILE_FUNCTION();

        std::cout << "adding bedrock..." << std::endl;

    }

    /// TODO: Adding more sophisticated cave generation techniques, like cellular automata or Voronoi diagrams, could make your cave systems more organic and interesting
    /// @brief add caves
    void addCaves()
    {
        PROFILE_FUNCTION();

        std::cout << "adding caves..." << std::endl;

        float caveScale = 0.5f;    // controls cave frequency
        float caveThreshold = 0.3f; // threshold for creating caves
        for (int y = 0; y < m_worldTilesY; ++y)
        {
            for (int x = 0; x < m_worldTilesX; ++x)
            {
                float caveNoise = m_noise.GetNoise(static_cast<float>(x) * caveScale, static_cast<float>(y) * caveScale);
                if (caveNoise > caveThreshold)
                {
                    m_tileMatrix[x][y] = NONE; // no tile in the tile layer, but still may have unique background on background layer matrix (in front of actual parallax background, like Terraria)
                }
            }
        }

        /// TODO: can look into different type of noise for this like rigid multifractal noise, can change thresholds for when a cave is made based on world y coord
    }

    /// TODO: could improve it by incorporating some horizontal variation and adding more diversity in terms of terrain features above the sea level
    void createSkyline()
    {
        PROFILE_FUNCTION();

        std::cout << "creating skyline..." << std::endl;

        // 1D noise for skyline along x-axis
        std::vector<int> terrainHeights(m_worldTilesX);
        float terrainDelta = 20.0f; // controls max deviation from sea level
        float noiseScale = 0.5f;
        float seaLevel = m_worldTilesY / 5; // number of tiles below the top of the screen
        for (int x = 0; x < m_worldTilesX; ++x)
        {
            float noiseVal = m_noise.GetNoise(static_cast<float>(x) * noiseScale, 0.0f); // [-1, 1]
            terrainHeights[x] = noiseVal * terrainDelta + seaLevel;  // [seaLevel - terrainDelta, seaLevel + terrainDelta]
        }

        /// TODO: another 1d noise sweep but moving tiles left and right to get some overhangs and things like that, Perlin best for this
        /// TODO: can do tons of passes, tweaks, whatever to make surface look like almost anything, look into it, this video gives the non-technical taste: https://www.youtube.com/watch?v=-POwgollFeY&ab_channel=DigiDigger, can do biomes like this by only applying some stuff to some areas 

        // remove tiles above terrain heights
        for (int x = 0; x < m_worldTilesX; ++x)
        {
            for (int y = 0; y < m_worldTilesY; ++y)
            {
                if (y < terrainHeights[x])
                {
                    m_tileMatrix[x][y] = NONE;
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
        m_tileMatrix = std::vector<std::vector<TileType>>(m_worldTilesX, std::vector<TileType>(m_worldTilesY));
    }

    void generateWorld()
    {
        PROFILE_FUNCTION();

        generateBaseLayer();
        createBlockPatches();
        addBedrock();
        addCaves();
        createSkyline();
    }

    std::vector<std::vector<TileType>> getTileMatrix()
    {
        return m_tileMatrix;
    }
};