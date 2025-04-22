// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Core
#include "Assets.hpp"

// Network
#include "network/NetworkManager.hpp"

// External libraries
#include <SFML/Graphics.hpp>

// C++ standard libraries
#include <map>
#include <string>
#include <memory>
#include <chrono>

class Scene; // forward declaration to resolve circular dependency, no need to include Scene.hpp here since GameEngine.hpp doesn't need to know the details

class GameEngine
{
    NetworkManager m_netManager;
    sf::RenderWindow m_window;
    Assets m_assets;
    std::string m_currentScene;
    std::map<std::string, std::shared_ptr<Scene>> m_sceneMap;
    bool m_running = true;

    void init();
    // void update(std::chrono::duration<long long, std::nano>& lag);
    void update();
    void sUserInput();
    std::shared_ptr<Scene> currentScene();

public:
    GameEngine();

    void addScene(const std::string& sceneName, std::shared_ptr<Scene> scene);
    void changeScene(const std::string& sceneName);
    void quit();
    void run();

    sf::RenderWindow& window();
    NetworkManager& getNetManager();
    Assets& assets();
    bool isRunning() const;
};
