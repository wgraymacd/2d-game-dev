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
    const int m_cellSizePixels = GlobalSettings::cellSizePixels; // cell size (pixels)
    const Vec2i m_worldMaxPixels = { m_cellSizePixels * m_worldMaxCells.x, m_cellSizePixels * m_worldMaxCells.y };

    // views and textures
    sf::View m_mainView = sf::View({ 0.0f, 0.0f }, GlobalSettings::windowSize.to<float>()); // center, size
    sf::RenderTexture m_tileTexture = sf::RenderTexture({ static_cast<unsigned int>(m_mainView.getSize().x / m_cellSizePixels), static_cast<unsigned int>(m_mainView.getSize().y / m_cellSizePixels) }); /// TODO: might need a plus one since we go from xMin through xMax
    sf::View m_miniMapView = sf::View({ 0.0f, 0.0f }, { m_cellSizePixels * 250.0f, m_cellSizePixels * 250.0f }); // center, size

    // entities
    EntityManager m_entityManager = EntityManager(m_worldMaxCells, m_cellSizePixels);
    Entity m_player, m_weapon; // commonly used
    PlayerConfig m_playerConfig;

    // rendering
    bool m_drawTextures = true;
    bool m_drawMinimap = true;
    bool m_drawCollision = false;

    // fps counter
    sf::Clock m_fpsClock;
    sf::Text m_fpsText = sf::Text(m_game.assets().getFont("font"));

    void init(); /// TODO: may add param here to differentiate between game types or something

    void loadGame(); /// TODO: may add param here to differentiate between game types or something
    void generateWorld();
    void spawnPlayer();
    void spawnBullet(Entity entity);
    void updateProjectiles(std::vector<Entity>& bullets);
    void playerTileCollisions(const std::vector<std::vector<Entity>>& tileMatrix);
    void projectileTileCollisions(std::vector<std::vector<Entity>>& tileMatrix, std::vector<Entity>& bullets);
    void projectilePlayerCollisions(std::vector<Entity>& players, std::vector<Entity>& bullets);
    void spawnRagdoll(Entity& player, Entity& bullet);
    Vec2f gridToMidPixel(const float gridX, const float gridY, const Entity entity);
    float generateRandomFloat(float min, float max);
    void findOpenTiles(int x, int y, const int minX, const int maxX, const int minY, const int maxY, const std::vector<std::vector<Entity>>& tileMatrix, std::vector<Vec2i>& openTiles, std::stack<Vec2i>& tileStack, std::vector<std::vector<bool>>& visited);

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
    void sCamera(); // entity: transform
    void sRender() override; // entity: animation, transform; tile: color

public:
    ScenePlay(GameEngine& game); /// TODO: may add param here to differentiate between game types or something
};