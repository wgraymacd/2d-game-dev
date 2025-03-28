// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "Scene.hpp"
#include "GameEngine.hpp"
#include "Action.hpp"
#include "network/NetworkData.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
// #include <chrono>

class SceneMenu : public Scene {
    std::string m_title;
    std::vector<std::string> m_menuStrings;
    sf::Text m_menuText = sf::Text(m_game.assets().getFont("font"));
    int m_selectedMenuIndex = 0;

    void init();

    void onEnd() override;
    // void updateState(std::chrono::duration<long long, std::nano>& lag) override;
    void updateState() override;
    void sRender() override;
    void sDoAction(const Action& action) override;

public:
    SceneMenu(GameEngine& gameEngine);
};
