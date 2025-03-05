// Copyright 2025, William MacDonald, All Rights Reserved.

#include "Globals.hpp"
#include "GameEngine.hpp"
#include "SceneMenu.hpp"
#include "Scene.hpp"
#include "Assets.hpp"

#include <string>
#include <memory>
#include <chrono>
#include <fstream>

/// TODO: consider multithreading, offload tasks like physics updates and asset loading to keep main game loop responsive
/// TODO: decouple frame rate from updates in main game loop
/// TODO: could introduce a more formal state management system for each scene to handle different modes (e.g., main menu, in-game, paused, game over) and manage transitions between them more gracefully
/// TODO: consider refactoring how the ActionMap is structured. A more efficient lookup mechanism (e.g., a hash map of actions to key/button codes) could improve performance and maintainability
/// TODO: Consider adding a stack of scenes, where you can "push" and "pop" scenes. This would allow for more advanced scenarios like pausing the game (by pushing a pause scene) or implementing menus, without directly replacing scenes
/// TODO: could implement an unordered map (std::unordered_map) for faster lookups. Since scene names are strings, and assuming they are frequently accessed, an unordered map would provide better performance for scene lookups
/// TODO: You might consider implementing a sf::Clock (or similar) to track delta time and simulation speed (m_simulationSpeed). This can ensure consistent updates regardless of the frame rate, allowing for frame-independent movement or animations

/// @brief constructs a new GameEngine by calling GameEngine::init
/// @param path the path to the asset configuration file
GameEngine::GameEngine(const std::string& path)
{
    init(path);
}

/// TODO: make window size dynamic, not hardcoded
/// @brief loads assets, creates window, and opens MENU scene
/// @param path the path to the asset configuration file
void GameEngine::init(const std::string& path)
{
    m_assets.loadFromFile(path);

    addScene("MENU", std::make_shared<SceneMenu>(*this));
}

/// @brief continuously calls GameEngine::update while the game is still running
void GameEngine::run()
{
    std::chrono::steady_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> lag(0);

    while (isRunning())
    {
        std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<long long, std::nano> elapsedTime = currentTime - lastTime;
        lastTime = currentTime;
        lag += elapsedTime;

        update(lag);
    }
}

/// @brief updates the game state with GameEngine::sUserInput and the active scene's update function
void GameEngine::update(std::chrono::duration<long long, std::nano>& lag)
{
    m_netManager.update();
    // currentScene()->updateState(lag);
}

/// @brief quits the game
void GameEngine::quit()
{
    m_running = false;
}

/// @brief gets a boolean representing the game state
/// @return true if game is running, false otherwise
bool GameEngine::isRunning() const
{
    return m_running;
}

/// @brief gets the current scene
/// @return shared pointer to a Scene object
std::shared_ptr<Scene> GameEngine::currentScene()
{
    return m_sceneMap[m_currentScene];
}

/// @brief gets all assets stored in the GameEngine object
/// @return Assets object that holds all information related to the game's assets
Assets& GameEngine::assets()
{
    return m_assets;
}

/// TODO: implement current scene ending
/// @brief adds a scene to the game
/// @param sceneName the name of the scene to be added (e.g., "MENU")
/// @param scene a new Scene object
/// @param endThisScene a boolean to control whether the current scene ends or not upon moving to the new scene
void GameEngine::addScene(const std::string& sceneName, std::shared_ptr<Scene> scene, bool endThisScene)
{
    m_sceneMap[sceneName] = scene;
    m_currentScene = sceneName;
}

/// TODO: implement current scene ending
/// @brief change to another active scene
/// @param sceneName name of the scene to change to (e.g., "PLAY")
/// @param endThisScene a boolean to control whether the current scene ends or not upon moving to the new scene
void GameEngine::changeScene(const std::string& sceneName, bool endThisScene)
{
    m_currentScene = sceneName;
}

/// @brief return the game's network manager
NetworkManager& GameEngine::getNetManager()
{
    return m_netManager;
}
