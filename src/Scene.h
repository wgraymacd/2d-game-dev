#pragma once

#include "Action.hpp"
#include <map>

class GameEngine; // DONT include GameEngine.h this because it created circular dependency with GameEngine.h, just let Scene.h know that GameEngine.h exists with this forward declaration

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

    // sf::Keyboard::A == sf::Mouse::Button::Left == 0 so need to distinguish between keyboard and mouse input
    void registerAction(int inputKey, const std::string &actionName, bool isMouseButton = false);

    void setPaused(bool paused);
};
