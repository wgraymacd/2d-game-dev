// Copyright 2025, William MacDonald, All Rights Reserved.

#include "Scene.hpp"

#include <string>
#include <map>

class GameEngine; // forward declaration

/// @brief constructs a new Scene object
/// @param game a reference to the game's main engine; required by derived classes of Scene to access top-level methods for Scene changing, adding, game quiting, etc.
Scene::Scene(GameEngine& game) : m_game(game) {}

const std::map<int, std::string>& Scene::getActionMap() const
{
    return m_actionMap;
}

/// @brief pauses the scene
/// @param paused true to pause, false to play
void Scene::setPaused(bool paused)
{
    m_paused = !m_paused;
}
