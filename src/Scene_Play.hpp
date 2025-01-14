#pragma once

#include "Scene.hpp"
#include "EntityManager.hpp"
#include "Vec2.hpp"
#include "GameEngine.hpp"

#include <SFML/Graphics.hpp>

#include <string>
#include <chrono>

class Scene_Play : public Scene
{
    struct PlayerConfig
    {
        float GX, GY, CW, CH, SX, SY, SM, GRAVITY; // grid pos, bounding box size, speed in X and Y and max, gravity
        std::string BA; // bullet animation
    };

protected:
    PlayerConfig m_playerConfig;
    EntityManager m_entityManager;
    Entity m_player = m_entityManager.addEntity("player");

    const Vec2i m_gridSize = { 20, 20 }; // cell size (pixels)
    sf::Text m_gridText = sf::Text(m_game.assets().getFont("PixelCowboy"));

    // Vec2f m_worldMin = {0.0f, 0.0f}; // top-left corner of world (this variables is not really necessary)
    Vec2i m_worldMax = { m_gridSize.x * 200, m_gridSize.y * 100 }; // bottom-right corner of world

    bool m_drawTextures = true;
    bool m_drawCollision = false;
    bool m_drawGrid = false;

    // fps counter
    sf::Clock m_fpsClock;
    sf::Text m_fpsText = sf::Text(m_game.assets().getFont("PixelCowboy"));

    void init(); /// TODO: may add param here to differentiate between game types or something

    void loadGame(); /// TODO: may add param here to differentiate between game types or something
    void generateWorld();

    void update(std::chrono::duration<long long, std::nano>& lag) override;
    void onEnd() override;
    void spawnPlayer();
    void spawnBullet(Entity entity);
    Vec2f gridToMidPixel(float x, float y, Entity entity);
    // Vec2f gridToPixel(float x, float y);

    void sMovement();
    void sStatus(); // lifespan, health, etc.
    void sCollision();
    void sDoAction(const Action& action) override;
    void sAnimation();
    void sAI(); // NPC behavior
    void sCamera();
    void sRender() override;

    void drawLine(const Vec2f& p1, const Vec2f& p2);

public:
    Scene_Play(GameEngine& game); /// TODO: may add param here to differentiate between game types or something
};