#pragma once

#include "Scene.hpp"
#include "EntityManager.hpp"
#include "Vec2.hpp"
#include "GameEngine.hpp"
#include "GlobalSettings.hpp"

#include <SFML/Graphics.hpp>

#include <string>
#include <chrono>

class ScenePlay : public Scene
{
    struct PlayerConfig
    {
        float GX, GY, CW, CH, SX, SY, SM, GRAVITY; // grid pos, bounding box size, speed in X and Y and max, gravity
        std::string BA; // bullet animation
    };

protected:
    // tile grid
    const Vec2ui m_worldMaxCells = GlobalSettings::worldMaxCells; // bottom-right corner of world (grid coords)
    const Vec2ui m_cellSizePixels = GlobalSettings::cellSizePixels; // cell size (pixels)
    const Vec2ui m_worldMaxPixels = { m_cellSizePixels.x * m_worldMaxCells.x, m_cellSizePixels.y * m_worldMaxCells.y };

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
    void playerTileCollisions(const std::vector<std::vector<Entity>>& tileMatrix);
    void bulletTileCollisions(const std::vector<std::vector<Entity>>& tileMatrix, const std::vector<Entity>& bullets);
    Vec2f gridToMidPixel(float x, float y, Entity entity);
    void update(std::chrono::duration<long long, std::nano>& lag) override;
    void onEnd() override;

    void sMovement();
    void sFiring();
    void sStatus(); // lifespan, health, etc.
    void sCollision();
    void sDoAction(const Action& action) override;
    void sAnimation();
    void sAI(); // NPC behavior
    void sCamera();
    void sRender() override;

    void drawLine(const Vec2f& p1, const Vec2f& p2);

public:
    ScenePlay(GameEngine& game); /// TODO: may add param here to differentiate between game types or something
};