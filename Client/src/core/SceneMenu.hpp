// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Core
#include "Scene.hpp"
#include "GameEngine.hpp"
#include "Action.hpp"

// Global
#include "NetworkData.hpp"

// External libraries
#include <SFML/Graphics.hpp>

// C++ standard libraries
#include <string>
#include <vector>
// #include <chrono>

class SceneMenu : public Scene
{
    std::string m_title;
    std::vector<std::string> m_menuStrings;
    sf::Text m_menuText = sf::Text(m_game.assets().getFont("Default"));

    size_t m_selectedMenuIndex = 0;

    void init();

    void onEnd() override;
    // void updateState(std::chrono::duration<long long, std::nano>& lag) override;
    void updateState() override;
    void updateFromNetwork();

    void sRender() override;
    void sDoAction(const Action& action) override;

public:
    SceneMenu(GameEngine& gameEngine);
};
