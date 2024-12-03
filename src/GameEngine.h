#pragma once

#include "Assets.hpp"

class Scene; // forward declaration to resolve circular dependency, no need to include Scene.h here since GameEngine.h doesn't need to know the details

using SceneMap = std::map<std::string, std::shared_ptr<Scene>>;

class GameEngine
{
protected:
    sf::RenderWindow m_window;
    Assets m_assets;
    std::string m_currentScene;
    SceneMap m_sceneMap;
    bool m_running = true;
    // size_t m_simulationSpeed = 1;
    // sf::Clock m_deltaClock;

    void init(const std::string &path);
    void update();
    void sUserInput();
    std::shared_ptr<Scene> currentScene();

public:
    GameEngine(const std::string &path);

    void addScene(const std::string &sceneName, std::shared_ptr<Scene> scene, bool endThisScene = false);
    void changeScene(const std::string &sceneName, bool endThisScene = false);
    void quit();
    void run();

    sf::RenderWindow &window();
    const Assets &assets() const;
    bool isRunning() const;
};