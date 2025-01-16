#pragma once

#include "Scene.hpp"
#include "GameEngine.hpp"
#include "Action.hpp"

#include <string>
#include <vector>
#include <chrono>

class SceneMenu : public Scene
{
    std::string m_title;
    std::vector<std::string> m_menuStrings;
    sf::Text m_menuText = sf::Text(m_game.assets().getFont("PixelCowboy"));
    size_t m_selectedMenuIndex = 0;

    void init();

    void onEnd() override;
    void update(std::chrono::duration<long long, std::nano>& lag) override;
    void sRender() override;
    void sDoAction(const Action& action) override;

public:
    SceneMenu(GameEngine& gameEngine);
};