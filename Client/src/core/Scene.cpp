// Copyright 2025, William MacDonald, All Rights Reserved.

// Core
#include "Scene.hpp"

// External libraries
#include <SFML/Graphics.hpp>

// C++ standard libraries
#include <string>
#include <map>

class GameEngine; // forward declaration

/// @brief constructs a new Scene object
/// @param game a reference to the game's main engine; required by derived classes of Scene to access top-level methods for Scene changing, adding, game quiting, etc.
Scene::Scene(GameEngine& game) : m_game(game) { }

/// @brief maps an input to an action name
/// @param input a key or mouse button
/// @param actionName the name of the action (e.g., "SHOOT")
/// @param isMouseButton boolean representing whether the input is a key or a mouse button
void Scene::registerAction(unsigned int input, const std::string& actionName, bool isMouseButton)
{
    m_actionMap[input + isMouseButton * sf::Keyboard::KeyCount] = actionName; // add constant offset to distinguish mouse clicks and keyboard clicks
}

const std::map<unsigned int, std::string>& Scene::getActionMap() const
{
    return m_actionMap;
}

/// @brief pauses the scene
/// @param paused true to pause, false to play
void Scene::setPaused()
{
    m_paused = !m_paused;
}
