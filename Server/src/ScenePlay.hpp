// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "Scene.hpp"
#include "EntityManager.hpp"
#include "Vec2.hpp"
#include "GameEngine.hpp"
#include "Globals.hpp"
#include "TileManager.hpp"

#include <string>
#include <chrono>

class ScenePlay : public Scene
{
    struct PlayerConfig
    {
        float CW, CH, SX, SY, SM, GRAVITY; // bounding box size, speed in X and Y and max, gravity
        std::string BA; // bullet animation
    };

protected:
    // tile grid
    const Vec2i m_worldMaxCells = GlobalSettings::worldMaxCells; // bottom-right corner of world (grid coords)
    const int m_cellSizePixels = GlobalSettings::cellSizePixels; // cell size (pixels)
    const Vec2i m_worldMaxPixels = { m_cellSizePixels * m_worldMaxCells.x, m_cellSizePixels * m_worldMaxCells.y };

    // entities
    EntityManager m_entityManager = EntityManager(m_worldMaxCells, m_cellSizePixels);
    Entity m_player, m_playerArmFront, m_playerArmBack, m_playerHead, m_weapon; // commonly used
    PlayerConfig m_playerConfig;

    // tiles
    TileManager m_tileManager;

    void init(); /// TODO: may add param here to differentiate between game types or something

    void loadGame(); /// TODO: may add param here to differentiate between game types or something
    void generateWorld();
    void spawnPlayer();
    void updateProjectiles(std::vector<Entity>& bullets);
    void projectileTileCollisions(std::vector<Tile>& tiles, std::vector<Entity>& bullets);
    void projectilePlayerCollisions(std::vector<Entity>& players, std::vector<Entity>& bullets);
    Entity spawnRagdollElement(const Vec2f& pos, const float angle, const Vec2i& boxSize, const Animation& animation);
    void createRagdoll(const Entity& entity, const Entity& cause);
    Vec2f gridToMidPixel(const float gridX, const float gridY, const Entity entity);
    float generateRandomFloat(float min, float max);
    void findOpenTiles(int x, int y, const int minX, const int maxX, const int minY, const int maxY, const std::vector<Tile>& tiles, std::vector<Vec2i>& openTiles, std::stack<Vec2i>& tileStack, std::vector<std::vector<bool>>& visited);
    std::vector<Vec2f> rayCast(const Vec2f& viewCenter, const Vec2f& viewSize, const std::vector<Vec2i>& openTiles, const Vec2f& origin, const std::vector<Tile>& tiles, int minX, int maxX, int minY, int maxY);
    // void propagateLight(sf::VertexArray& blocks, int maxDepth, int currentDepth, const Vec2i& startCoord, Vec2i currentCoord, int minX, int maxX, int minY, int maxY);
    // void addBlock(sf::VertexArray& blocks, const int xGrid, const int yGrid, const sf::Color& c);

    void updateState(std::chrono::duration<long long, std::nano>& lag) override;
    void onEnd() override;
    // void resizeView(const Vec2f& size); /// TODO: could also have no arguments and go check globalsettings windowsize since updated first, this func used if resizing needs to be specific to each scene

    /// TODO: group these to best handle single components for multiple entities at once
    void sObjectMovement(); // entity: state, input, transform
    void sObjectCollision(); // entity: transform, state, bounding box, input, damage; tile: 
    void sProjectiles(); // entity: input, firerate, transform, invincibility, damage, health; tile: health, type
    void sStatus(); // entity: lifespan, invincibility
    void sDoAction(const Action& action) override; // entity: input
    void sAnimation(); // entity: animation
    void sAI(); // 

public:
    ScenePlay(GameEngine& game); /// TODO: may add param here to differentiate between game types or something
};