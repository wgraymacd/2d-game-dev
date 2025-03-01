// Copyright 2025, William MacDonald, All Rights Reserved.

/// TODO: to make world gen and general game logic and maintenence easier, remove all entity-related stuff from tiles, just put all data in tile matrix (still have to store it all and fetch it from the memory pool anyway, waste to have tile matrix and pool, just separate it from ECS, can then have vision block info, yada, etc.); do this for background and decor as well if needed (or have some var or something that determines if, upon destorying a tile, a background should spawn in it's place, then still keep only one matrix and less data)


#pragma once

#include <string>
#include <vector>

#include "FastNoiseLite.h"
#include "TileType.hpp"
#include "StructureTypes.hpp"

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
    int m_dirtToStone = 2;
    int m_seed = std::time(0);

    FastNoiseLite m_noise;

    std::vector<TileType> m_tileTypes;

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
                if (y <= m_worldTilesY / m_dirtToStone)
                {
                    m_tileTypes[x * m_worldTilesY + y] = DIRT;
                }
                else
                {
                    m_tileTypes[x * m_worldTilesY + y] = STONE;
                }
            }
        }
    }

    /// @brief add some dirt in stone and some stone in dirt
    void createBlockPatches()
    {
        PROFILE_FUNCTION();

        std::cout << "creating block patches..." << std::endl;

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

        for (int y = 0; y < m_worldTilesY; ++y)
        {
            for (int x = 0; x < m_worldTilesX; ++x)
            {
                float patchNoise = m_noise.GetNoise(static_cast<float>(x), static_cast<float>(y));
                if (patchNoise > stoneThreshold && y <= m_worldTilesY / m_dirtToStone)
                {
                    m_tileTypes[x * m_worldTilesY + y] = STONE;
                }
                else if (patchNoise > dirtThreshold && y > m_worldTilesY / m_dirtToStone)
                {
                    m_tileTypes[x * m_worldTilesY + y] = DIRT;
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
        for (int y = 0; y < m_worldTilesY; ++y)
        {
            for (int x = 0; x < m_worldTilesX; ++x)
            {
                float caveNoise = m_noise.GetNoise(static_cast<float>(x), static_cast<float>(y * 1.5)); // TODO: mess with multiplying x or y by scalar to stretch or squish noise along an axis
                if (caveNoise * (1 + 0.5f * y / m_worldTilesY) > caveThreshold)
                {
                    TileType& type = m_tileTypes[x * m_worldTilesY + y];
                    if (type == DIRT)
                    {
                        type = DIRTWALL;
                    }
                    else if (type == STONE)
                    {
                        type = STONEWALL;
                    }
                }
            }
        }

        /// TODO: can look into different type of noise for this like rigid multifractal noise, can change thresholds for when a cave is made based on world y coord
    }

    void addBuildings()
    {
        StructureTypes structures;
        int numberOfBuildings = (m_worldTilesX * m_worldTilesY) / 50000;
        for (int i = 0; i < numberOfBuildings; ++i)
        {
            // int structType = rand() % numberOfBuildings;
            int xPos = rand() % m_worldTilesX; // left
            int yPos = rand() % m_worldTilesY; // top

            int structSizeX = structures.hallway.size();
            int structSizeY = structures.hallway[0].size();

            for (int x = xPos; x < xPos + structSizeX; ++x)
            {
                if (x >= 0 && x < m_worldTilesX)
                {
                    for (int y = yPos; y < yPos + structSizeY; ++y)
                    {
                        if (y >= 0 && y < m_worldTilesY)
                        {
                            m_tileTypes[x * m_worldTilesY + y] = structures.hallway[x - xPos][y - yPos];
                        }
                    }
                }
            }

            // int structType = rand() % numberOfBuildings;
            xPos += structSizeX - 1;

            for (int x = xPos; x < xPos + structSizeX; ++x)
            {
                if (x >= 0 && x < m_worldTilesX)
                {
                    for (int y = yPos; y < yPos + structSizeY; ++y)
                    {
                        if (y >= 0 && y < m_worldTilesY)
                        {
                            m_tileTypes[x * m_worldTilesY + y] = structures.hallway[x - xPos][y - yPos];
                        }
                    }
                }
            }

            yPos += structSizeY - 1;

            for (int x = xPos; x < xPos + structSizeX; ++x)
            {
                if (x >= 0 && x < m_worldTilesX)
                {
                    for (int y = yPos; y < yPos + structSizeY; ++y)
                    {
                        if (y >= 0 && y < m_worldTilesY)
                        {
                            m_tileTypes[x * m_worldTilesY + y] = structures.hallway[x - xPos][y - yPos];
                        }
                    }
                }
            }

            yPos += structSizeY - 1;

            for (int x = xPos; x < xPos + structSizeX; ++x)
            {
                if (x >= 0 && x < m_worldTilesX)
                {
                    for (int y = yPos; y < yPos + structSizeY; ++y)
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

        std::cout << "creating skyline..." << std::endl;

        // 1D noise for skyline along x-axis
        std::vector<int> terrainHeight(m_worldTilesX);
        float terrainDelta = 50.0f; // controls max deviation from sea level
        float noiseScale = 1.0f;
        float seaLevel = m_worldTilesY / 5; // number of tiles below the top of the screen
        for (int x = 0; x < m_worldTilesX; ++x)
        {
            float noiseVal = m_noise.GetNoise(static_cast<float>(x) * noiseScale, 0.0f);
            float extraNoise = m_noise.GetNoise(0.0f, static_cast<float>(x) * noiseScale);
            noiseVal += extraNoise;
            terrainHeight[x] = noiseVal * terrainDelta + seaLevel;

            for (int y = 0; y < m_worldTilesY; ++y)
            {
                if (y < terrainHeight[x])
                {
                    m_tileTypes[x * m_worldTilesY + y] = NONE;
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

public:
    /// @brief constructs WorldGenerator object, defines noise type as Perlin
    /// @param width the width of the game world (in grid units)
    /// @param height the height of the game world (in grid units)
    WorldGenerator(int numTilesX, int numTilesY)
        : m_worldTilesX(numTilesX), m_worldTilesY(numTilesY)
    {
        m_noise.SetSeed(m_seed);
        m_tileTypes = std::vector<TileType>(m_worldTilesX * m_worldTilesY);
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
};