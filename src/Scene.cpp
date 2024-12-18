#include "Scene.h"
#include "GameEngine.h"

#include <string>
#include <map>

Scene::Scene(GameEngine &game)
    : m_game(game) {}

void Scene::registerAction(int input, const std::string &actionName, bool isMouseButton)
{
    m_actionMap[input + isMouseButton * sf::Keyboard::KeyCount] = actionName; // add constant offset to distinguish mouse clicks and keyboard clicks
}

const std::map<int, std::string> &Scene::getActionMap() const
{
    return m_actionMap;
}

void Scene::setPaused(bool paused)
{
    m_paused = !m_paused;
}
