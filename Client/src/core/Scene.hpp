// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Core
#include "Action.hpp"

// Global
#include "NetworkData.hpp"

// C++ standard libraries
#include <map>
#include <vector>

class GameEngine;

class Scene
{
protected:
    GameEngine& m_game;
    int m_currentFrame = 0;
    bool m_hasEnded = false;
    bool m_paused = false;
    std::map<unsigned int, std::string> m_actionMap;

public:
    Scene(GameEngine& game);
    virtual ~Scene() = default; // Virtual destructor

    // virtual void updateState(std::chrono::duration<long long, std::nano>& lag) = 0;
    virtual void updateState() = 0;
    virtual void onEnd() = 0;
    virtual void sRender() = 0;
    virtual void sDoAction(const Action& action) = 0;

    const std::map<unsigned int, std::string>& getActionMap() const;

    // sf::Keyboard::A == sf::Mouse::Button::Left == 0 so need to distinguish between keyboard and mouse input
    void registerAction(unsigned int inputKey, const std::string& actionName, bool isMouseButton = false);

    void setPaused();
};
