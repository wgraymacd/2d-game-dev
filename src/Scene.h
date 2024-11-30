#pragma once

#include "Action.hpp"
#include <map>

// #include "GameEngine.h" DONT include this because it created circular dependency with GameEngine.h, just let Scene.h know that GameEngine.h exists
class GameEngine;

class Scene
{
protected:
    GameEngine &m_game;
    size_t m_currentFrame = 0;
    bool m_hasEnded = false;
    bool m_paused = false;
    std::map<int, std::string> m_actionMap;

public:
    Scene(GameEngine &game);

    virtual void update() = 0;
    virtual void onEnd() = 0;
    virtual void sRender() = 0;
    virtual void sDoAction(const Action &action) = 0;

    const std::map<int, std::string> &getActionMap() const;

    void registerAction(int inputKey, const std::string &actionName);
    void setPaused(bool paused);
};
