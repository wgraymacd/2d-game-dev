// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Core
#include "Scene.hpp"
#include "EntityManager.hpp"
#include "GameEngine.hpp"

// Physics
#include "physics/Vec2.hpp"

// Utility
#include "utility/ClientGlobals.hpp"

// World
#include "world/TileManager.hpp"

// External libraries
#include <SFML/Graphics.hpp>

// C++ standard libraries
#include <string>
#include <chrono>

class ScenePlay : public Scene
{

public:

    ScenePlay(GameEngine& game, int worldSeed); /// TODO: may be a good idea to add new scenes for new game types, or add parameter to distinguish them if not

protected:

    struct PlayerConfig
    {
        float CW, CH, SX, SY, SM, GRAVITY; // speed in X and Y and max, gravity
        std::string BA; // bullet animation
    };

    const Vec2i m_worldMaxCells { Settings::worldMaxCellsX, Settings::worldMaxCellsY };
    const int m_cellSizePixels = Settings::cellSizePixels;
    const Vec2i m_worldMaxPixels { m_cellSizePixels * m_worldMaxCells.x,
                                   m_cellSizePixels * m_worldMaxCells.y };

    sf::View m_mainView = sf::View({ 0.0f, 0.0f }, sf::Vector2f(Settings::windowSizeX, Settings::windowSizeY)); // center, size
    // sf::RenderTexture m_tileTexture = sf::RenderTexture({ static_cast<unsigned int>(m_mainView.getSize().x / m_cellSizePixels), static_cast<unsigned int>(m_mainView.getSize().y / m_cellSizePixels) }); /// TODO: might need a plus one since we go from xMin through xMax
    sf::View m_miniMapView = sf::View({ 0.0f, 0.0f }, sf::Vector2f(m_worldMaxCells.x, m_worldMaxCells.y) * 2.0f); // center, size

    // Entities
    EntityManager m_entityManager;
    Entity m_player, m_weapon; // commonly used
    Entity m_head, m_torso, m_leftUpperArm, m_leftForearm, m_rightUpperArm, m_rightForearm, m_leftHandBack, m_leftHandFront, m_rightHandBack, m_rightHandFront, m_leftThigh, m_rightThigh, m_leftCalf, m_rightCalf, m_leftFoot, m_rightFoot; // body parts
    PlayerConfig m_playerConfig;

    // Tiles
    TileManager m_tileManager;

    // Rendering
    bool m_drawTextures = true;
    bool m_drawMinimap = true;
    bool m_drawCollision = false;

    // FPS counter
    sf::Clock m_fpsClock;
    sf::Text m_fpsText = sf::Text(m_game.assets().getFont("Default"));

    void init(); /// TODO: may add param here to differentiate between game types or something

    void loadGame(); /// TODO: may add param here to differentiate between game types or something
    void generateWorld(int worldSeed);
    void spawnPlayer();
    void spawnBullet(Entity entity);
    void updateProjectiles(std::vector<Entity>& bullets);
    void playerTileCollisions(const std::vector<Tile>& tiles);
    void projectileTileCollisions(std::vector<Tile>& tiles, std::vector<Entity>& bullets);
    void projectilePlayerCollisions(std::vector<Entity>& players, std::vector<Entity>& bullets);
    Entity spawnRagdollElement(const Vec2f& pos, float angle, const Vec2f& boxSize, const Animation& animation);
    void createRagdoll(const Entity& entity, const Entity& cause);
    Vec2f gridToMidPixel(float gridX, float gridY, Entity entity);
    void findOpenTiles(int x, int y, int minX, int maxX, int minY, int maxY, const std::vector<Tile>& tiles, std::vector<Vec2i>& openTiles, std::vector<Vec2i>& tileStack, std::vector<char>& visited);
    std::vector<Vec2f> rayCast(const Vec2f& viewCenter, const Vec2f& viewSize, const std::vector<Vec2i>& openTiles, const Vec2f& origin, const std::vector<Tile>& tiles, int minX, int maxX, int minY, int maxY);
    // void propagateLight(sf::VertexArray& blocks, int maxDepth, int currentDepth, const Vec2i& startCoord, Vec2i currentCoord, int minX, int maxX, int minY, int maxY);
    void addBlock(sf::VertexArray& blocks, int xGrid, int yGrid, const sf::Color& c);

    // void updateState(std::chrono::duration<long long, std::nano>& lag) override;
    void updateState() override;
    void onEnd() override;
    // void resizeView(const Vec2f& size); /// TODO: could also have no arguments and go check globalsettings windowsize since updated first, this func used if resizing needs to be specific to each scene

    /// TODO: group these to best handle single components for multiple entities at once
    void sNetwork(); // get net data and create entities for received spawn requests
    void sObjectMovement(); // entity: state, input, transform
    void sObjectCollision(); // entity: transform, state, bounding box, input, damage; tile:
    void sProjectiles(); // entity: input, firerate, transform, invincibility, damage, health; tile: health, type
    void sStatus(); // entity: lifespan, invincibility
    void sDoAction(const Action& action) override; // entity: input
    void sAnimation(); // entity: animation
    void sAI(); //
    void sCamera(); // entity: transform
    void sRender() override; // entity: animation, transform; tile: color

};
