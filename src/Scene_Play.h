#pragma once

#include "Scene.h"
#include "EntityManager.hpp"

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
    bool m_drawTextures = true;
    bool m_drawCollision = false;
    bool m_drawGrid = false;
    const Vec2f m_gridSize = {40, 40}; // cell size
    sf::Text m_gridText;

    void init(const std::string &levelPath);

    void loadLevel(const std::string &filename);

    void update() override;
    void onEnd() override;
    void spawnPlayer();
    void spawnBullet(std::shared_ptr<Entity> entity);

    Vec2f gridToMidPixel(float x, float y, std::shared_ptr<Entity> entity);
    std::shared_ptr<Entity> player();

    void sMovement();
    void sLifespan();
    void sCollision();
    void sDoAction(const Action &action) override;
    void sAnimation();
    void sGUI();
    void sRender() override;

    void drawLine(const Vec2f &p1, const Vec2f &p2);

public:
    Scene_Play(GameEngine &game, const std::string &levelPath);
};