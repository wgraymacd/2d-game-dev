#pragma once

#include "Scene.h"
#include "EntityManager.hpp"
#include "Vec2.hpp"
#include "GameEngine.h"

#include <SFML/Graphics.hpp>

#include <string>
#include <memory>

class Scene_Play : public Scene
{
    struct PlayerConfig
    {
        float GX, GY, CW, CH, SX, SY, SM, GRAVITY; // grid pos, bounding box size, speed in X and Y and max, gravity
        std::string BA; // bullet animation
    };

protected:
    std::string m_levelPath;
    PlayerConfig m_playerConfig;
    EntityManager m_entityManager;
    std::shared_ptr<Entity> m_player;

    const Vec2i m_gridSize = {20, 20}; // cell size (pixels)
    sf::Text m_gridText;

    // Vec2f m_worldMin = {0.0f, 0.0f}; // top-left corner of world (this variables is not really necessary)
    Vec2i m_worldMax = {m_gridSize.x * 100, m_gridSize.y * 50}; // bottom-right corner of world

    bool m_drawTextures = true;
    bool m_drawCollision = false;
    bool m_drawGrid = false;

    // fps counter
    sf::Clock m_fpsClock;
    sf::Text m_fpsText;

    void init(const std::string &levelPath);

    void loadLevel(const std::string &filename);
    void generateWorld();

    void update() override;
    void onEnd() override;
    void spawnPlayer();
    void spawnBullet(std::shared_ptr<Entity> entity);
    void spawnMelee(std::shared_ptr<Entity> entity);
    Vec2f gridToMidPixel(float x, float y, std::shared_ptr<Entity> entity);
    // Vec2f gridToPixel(float x, float y);

    void sMovement();
    void sStatus(); // lifespan and invincibility
    void sCollision();
    void sDoAction(const Action &action) override;
    void sAnimation();
    void sAI(); // NPC behavior
    void sCamera(); // room vs center of player vs anything I might want
    void sRender() override;

    void drawLine(const Vec2f &p1, const Vec2f &p2);

public:
    Scene_Play(GameEngine &game, const std::string &levelPath);
};