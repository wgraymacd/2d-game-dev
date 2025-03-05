// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "Assets.hpp"
#include "NetworkManager.hpp"

#include <map>
#include <string>
#include <memory>
#include <chrono>

class Scene; // forward declaration to resolve circular dependency, no need to include Scene.hpp here since GameEngine.hpp doesn't need to know the details

class GameEngine
{
    NetworkManager m_netManager;
    Assets m_assets;
    std::string m_currentScene;
    std::map<std::string, std::shared_ptr<Scene>> m_sceneMap;
    bool m_running = true;

    void init(const std::string& path);
    void update(std::chrono::duration<long long, std::nano>& lag);
    std::shared_ptr<Scene> currentScene();

public:
    GameEngine(const std::string& path);

    void addScene(const std::string& sceneName, std::shared_ptr<Scene> scene, bool endThisScene = false);
    void changeScene(const std::string& sceneName, bool endThisScene = false);
    void quit();
    void run();

    NetworkManager& getNetManager();
    Assets& assets();
    bool isRunning() const;
};