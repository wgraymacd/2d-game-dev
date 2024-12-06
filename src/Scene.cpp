#include "Scene.h"
#include "GameEngine.h"

#include <string>
#include <map>

Scene::Scene(GameEngine &game)
    : m_game(game) {}

void Scene::registerAction(int inputKey, const std::string &actionName)
{
    m_actionMap[inputKey] = actionName;
}

const std::map<int, std::string> &Scene::getActionMap() const
{
    return m_actionMap;
}

void Scene::setPaused(bool paused)
{
    m_paused = !m_paused;
}
