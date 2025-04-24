// Copyright 2025, William MacDonald, All Rights Reserved.

/// TODO: to make world gen and general game logic and maintenence easier, remove all entity-related stuff from tiles, just put all data in tile matrix (still have to store it all and fetch it from the memory pool anyway, waste to have tile matrix and pool, just separate it from ECS, can then have vision block info, yada, etc.); do this for background and decor as well if needed (or have some var or something that determines if, upon destorying a tile, a background should spawn in it's place, then still keep only one matrix and less data)


#pragma once

// World
#include "FastNoiseLite.h"
#include "TileType.hpp"
#include "StructureTypes.hpp"

// Global
#include "Timer.hpp"
#include "Random.hpp"

// C++ standard libraries
#include <string>
#include <vector>
#include <iostream>

/// TODO: add biomes with specific rules for generation, could even define temp, humidity, etc. and calc tree density or water or weather or anything from them
/// TODO: enum for tile types
/// TODO: could add world evolution if I want people to be on same map for long time
/// NOTE: if I use a certain seed, shit never changes, so can always get back to the same world
class WorldGenerator
{
public:

    WorldGenerator(size_t numTilesX, size_t numTilesY, int worldSeed)
        : m_worldTilesX(numTilesX), m_worldTilesY(numTilesY), m_seed(worldSeed)
    {
        m_noise.SetSeed(m_seed);
        m_tileTypes.resize(m_worldTilesX * m_worldTilesY);
    }

    void generateWorld()
    {
        PROFILE_FUNCTION();

        generateBaseLayer();
        createBlockPatches();
        // addBedrock();
        addCaves();
        addBuildings();
        // createSkyline();
    }

    std::vector<TileType>& getTileTypes()
    {
        return m_tileTypes;
    }

private:

    size_t m_worldTilesX;
    size_t m_worldTilesY;

    float m_dirtToStone = 0.5f; // point (0 to 1) at which base layer switches from dirt to stone

    int m_seed;

    FastNoiseLite m_noise;

    std::vector<TileType> m_tileTypes;

    /// TODO: try chunk-based storage for rendering and everything, but will use vector of pairs instead for now
    // std::unordered_map<std::pair<int, int>, std::vector<std::vector<std::string>>> chunks;
    // accessed like chunks[{chunkX, chunkY}][localX][localY] = ...;

    /// @brief lay out dirt and stone layer, filling m_tilePositions
    void generateBaseLayer()
    {
        PROFILE_FUNCTION();

        std::cout << "Creating base layer..." << std::endl;

        for (size_t y = 0; y < m_worldTilesY; ++y)
        {
            for (size_t x = 0; x < m_worldTilesX; ++x)
            {
                if (y <= m_worldTilesY * m_dirtToStone)
                {
                    m_tileTypes[x * m_worldTilesY + y] = TileType::DIRT;
                }
                else
                {
                    m_tileTypes[x * m_worldTilesY + y] = TileType::STONE;
                }
            }
        }
    }

    /// @brief add some dirt in stone and some stone in dirt
    void createBlockPatches()
    {
        PROFILE_FUNCTION();

        std::cout << "Creating block patches..." << std::endl;

        m_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        m_noise.SetFrequency(0.02f);
        m_noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
        m_noise.SetFractalOctaves(3);
        m_noise.SetFractalLacunarity(1.29f);
        m_noise.SetFractalGain(1.03f);
        m_noise.SetFractalWeightedStrength(-0.45f);
        m_noise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2Reduced);
        m_noise.SetDomainWarpAmp(12.5f);

        float dirtThreshold = 0.6f; // threshold for creating dirt vein
        float stoneThreshold = 0.8f; // threshold for creating stone patch

        for (size_t y = 0; y < m_worldTilesY; ++y)
        {
            for (size_t x = 0; x < m_worldTilesX; ++x)
            {
                float patchNoise = m_noise.GetNoise(static_cast<float>(x), static_cast<float>(y));
                if (patchNoise > stoneThreshold && y <= m_worldTilesY * m_dirtToStone)
                {
                    m_tileTypes[x * m_worldTilesY + y] = TileType::STONE;
                }
                else if (patchNoise > dirtThreshold && y > m_worldTilesY * m_dirtToStone)
                {
                    m_tileTypes[x * m_worldTilesY + y] = TileType::DIRT;
                }
            }
        }
    }

    /// TODO:
    /// @brief add other things like bedrock veins
    void addBedrock()
    {
        PROFILE_FUNCTION();

        std::cout << "Adding bedrock..." << std::endl;

    }

    /// TODO: Adding more sophisticated cave generation techniques, like cellular automata or Voronoi diagrams, could make your cave systems more organic and interesting
    /// @brief add caves
    void addCaves()
    {
        PROFILE_FUNCTION();

        std::cout << "Adding caves..." << std::endl;

        // could do same as block patches but with m_noise.SetFrequency(0.01f) and m_noise.SetSeed(m_seed + 1)

        // or could do this
        m_noise = FastNoiseLite();
        m_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        m_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        m_noise.SetSeed(m_seed);
        m_noise.SetFractalGain(0.55f);
        // m_noise.SetFractalLacunarity(2.2f);
        // m_noise.SetFractalWeightedStrength(-0.3f);
        m_noise.SetFractalOctaves(4);

        float caveThreshold = 0.05f; // threshold for creating caves
        for (size_t y = 0; y < m_worldTilesY; ++y)
        {
            for (size_t x = 0; x < m_worldTilesX; ++x)
            {
                float caveNoise = m_noise.GetNoise(static_cast<float>(x), y * 1.5f);
                if (caveNoise * (1.0f + 0.5f * y / m_worldTilesY) > caveThreshold)
                {
                    TileType& type = m_tileTypes[x * m_worldTilesY + y];
                    if (type == TileType::DIRT)
                    {
                        type = TileType::DIRTWALL;
                    }
                    else if (type == TileType::STONE)
                    {
                        type = TileType::STONEWALL;
                    }
                }
            }
        }

        /// TODO: can look into different type of noise for this like rigid multifractal noise, can change thresholds for when a cave is made based on world y coord
    }

    void addBuildings()
    {
        std::cout << "Adding buildings..." << std::endl;

        StructureTypes structures;
        int numberOfBuildings = static_cast<int>(m_worldTilesX * m_worldTilesY) / 50000;
        for (int i = 0; i < numberOfBuildings; ++i)
        {
            // int structType = random % numberOfBuildings;
            size_t xPos = Random::getIntegral(0uz, m_worldTilesX); // left
            size_t yPos = Random::getIntegral(0uz, m_worldTilesY); // top

            size_t structSizeX = structures.hallway.size();
            size_t structSizeY = structures.hallway[0].size();

            for (size_t x = xPos; x < xPos + structSizeX; ++x)
            {
                if (x >= 0 && x < m_worldTilesX)
                {
                    for (size_t y = yPos; y < yPos + structSizeY; ++y)
                    {
                        if (y >= 0 && y < m_worldTilesY)
                        {
                            m_tileTypes[x * m_worldTilesY + y] = structures.hallway[x - xPos][y - yPos];
                        }
                    }
                }
            }

            // int structType = random % numberOfBuildings;
            xPos += structSizeX - 1;

            for (size_t x = xPos; x < xPos + structSizeX; ++x)
            {
                if (x >= 0 && x < m_worldTilesX)
                {
                    for (size_t y = yPos; y < yPos + structSizeY; ++y)
                    {
                        if (y >= 0 && y < m_worldTilesY)
                        {
                            m_tileTypes[x * m_worldTilesY + y] = structures.hallway[x - xPos][y - yPos];
                        }
                    }
                }
            }

            yPos += structSizeY - 1;

            for (size_t x = xPos; x < xPos + structSizeX; ++x)
            {
                if (x >= 0 && x < m_worldTilesX)
                {
                    for (size_t y = yPos; y < yPos + structSizeY; ++y)
                    {
                        if (y >= 0 && y < m_worldTilesY)
                        {
                            m_tileTypes[x * m_worldTilesY + y] = structures.hallway[x - xPos][y - yPos];
                        }
                    }
                }
            }

            yPos += structSizeY - 1;

            for (size_t x = xPos; x < xPos + structSizeX; ++x)
            {
                if (x >= 0 && x < m_worldTilesX)
                {
                    for (size_t y = yPos; y < yPos + structSizeY; ++y)
                    {
                        if (y >= 0 && y < m_worldTilesY)
                        {
                            m_tileTypes[x * m_worldTilesY + y] = structures.hallway[x - xPos][y - yPos];
                        }
                    }
                }
            }
        }
    }

    /// TODO: could improve it by incorporating some horizontal variation and adding more diversity in terms of terrain features above the sea level
    void createSkyline()
    {
        PROFILE_FUNCTION();

        std::cout << "Creating skyline..." << std::endl;

        // 1D noise for skyline along x-axis
        std::vector<unsigned int> terrainHeights(m_worldTilesX); // lighter on memory yet still unsigned, @todo: consider using uint16_t or uint8_t for smaller heights and making its max val a global bound on world size in Globals.hpp, or std::array if fixed size wherever possible

        float terrainDelta = 50.0f; // controls max deviation from sea level
        float noiseScale = 1.0f;
        float seaLevel = m_worldTilesY / 5.0f; // number of tiles below the top of the screen

        for (size_t x = 0; x < m_worldTilesX; ++x)
        {
            float noiseVal = m_noise.GetNoise(x * noiseScale, 0.0f);
            float extraNoise = m_noise.GetNoise(0.0f, x * noiseScale);
            noiseVal += extraNoise;
            terrainHeights[x] = static_cast<unsigned int>(noiseVal * terrainDelta + seaLevel);

            for (size_t y = 0; y < m_worldTilesY; ++y)
            {
                if (y < terrainHeights[x])
                {
                    m_tileTypes[x * m_worldTilesY + y] = TileType::NONE;
                }
            }
        }

        /// TODO: another 1d noise sweep but moving tiles left and right to get some overhangs and things like that, Perlin best for this
        /// TODO: can do tons of passes, tweaks, whatever to make surface look like almost anything, look into it, this video gives the non-technical taste: https://www.youtube.com/watch?v=-POwgollFeY&ab_channel=DigiDigger, can do biomes like this by only applying some stuff to some areas
        /// TODO: do I need to do one to the left and then one to the right, shifting right and then left respectively so I don't get weird shit?
        // noiseScale = 5.0f;
        // for (int x = 0; x < m_worldTilesX; ++x)
        // {
        //     float noiseVal = m_noise.GetNoise(static_cast<float>(x) * noiseScale, 0.0f);
        //     int shift = noiseVal * terrainDelta;
        //     int y = terrainHeight[x];

        //     // if (shift > 0)
        //     // {
        //     //     int iMax = std::max(x + shift, m_worldTilesX - 2);
        //     //     for (int i = x; i < iMax; ++i)
        //     //     {
        //     //         // move tile to the right by one
        //     //         if (m_tileTypes[i + 1][y])
        //     //         {
        //     //             m_tileTypes[i + 1][y] = m_tileTypes[i][y];
        //     //             m_tileTypes[i][y] = NONE;
        //     //         }
        //     //         else
        //     //         {
        //     //             m_tileTypes[i + 1][y] = m_tileTypes[i][y];
        //     //             m_tileTypes[i][y] = NONE;
        //     //         }
        //     //     }
        //     // }
        //     // else if (shift < 0)
        //     // {
        //     //     int iMin = std::min(x + shift, 1);
        //     //     for (int i = x; i > iMin; ++i)
        //     //     {

        //     //     }
        //     // }
        // }
    }
};
