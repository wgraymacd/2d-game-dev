#pragma once

#include "Scene.hpp"
#include "EntityManager.hpp"
#include "Vec2.hpp"
#include "GameEngine.hpp"
#include "Globals.hpp"

#include <SFML/Graphics.hpp>

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
    const Vec2i m_cellSizePixels = GlobalSettings::cellSizePixels; // cell size (pixels)
    const Vec2i m_worldMaxPixels = { m_cellSizePixels.x * m_worldMaxCells.x, m_cellSizePixels.y * m_worldMaxCells.y };

    // views
    sf::View m_mainView = sf::View({ 0.0f, 0.0f }, GlobalSettings::windowSize.to<float>()); // center, size
    sf::View m_miniMapView = sf::View({ 0.0f, 0.0f }, { m_cellSizePixels.x * 250.0f, m_cellSizePixels.y * 250.0f }); // center, size

    // entities
    EntityManager m_entityManager = EntityManager(m_worldMaxCells, m_cellSizePixels);
    Entity m_player = m_entityManager.addEntity("player");
    Entity m_weapon = m_entityManager.addEntity("weapon");
    PlayerConfig m_playerConfig;

    // rendering
    bool m_drawTextures = true;
    bool m_drawMinimap = true;
    bool m_drawCollision = false;
    bool m_drawGrid = false;

    // fps counter
    sf::Clock m_fpsClock;
    sf::Text m_fpsText = sf::Text(m_game.assets().getFont("PixelCowboy"));

    // grid text
    sf::Text m_gridText = sf::Text(m_game.assets().getFont("PixelCowboy"));


    void init(); /// TODO: may add param here to differentiate between game types or something

    void loadGame(); /// TODO: may add param here to differentiate between game types or something
    void generateWorld();
    void spawnPlayer();
    void spawnBullet(Entity entity);
    void updateProjectiles(std::vector<Entity>& bullets);
    void playerTileCollisions(const std::vector<std::vector<Entity>>& tileMatrix);
    void projectileTileCollisions(std::vector<std::vector<Entity>>& tileMatrix, std::vector<Entity>& bullets);
    Vec2f gridToMidPixel(const float gridX, const float gridY, const Entity entity);
    float generateRandomFloat(float min, float max);
    void findOpenTiles(int x, int y, const int minX, const int maxX, const int minY, const int maxY, const std::vector<std::vector<Entity>>& tileMatrix, std::vector<Vec2i>& openTiles, std::stack<Vec2i>& tileStack, std::vector<std::vector<bool>>& visited);

    void updateState(std::chrono::duration<long long, std::nano>& lag) override;
    void onEnd() override;
    // void resizeView(const Vec2f& size); /// TODO: could also have no arguments and go check globalsettings windowsize since updated first, this func used if resizing needs to be specific to each scene

    /// TODO: group these to best handle single components for multiple entities at once
    void sObjectMovement(); // read player state and input, modify player transform
    void sObjectCollision(); // player transform, state, bounding box, and input, tile transform and health, bullet transform and damage
    void sProjectiles(); // reads player input and weapon firerate, spawns bullets
    void sLifespan(); // lifespan
    void sDoAction(const Action& action) override; // modifies player input
    void sAnimation(); // animation
    void sAI(); // 
    void sCamera(); // 
    void sRender() override; // animations, transforms

    void drawLine(const Vec2f& p1, const Vec2f& p2);

public:
    ScenePlay(GameEngine& game); /// TODO: may add param here to differentiate between game types or something
};