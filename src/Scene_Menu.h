#pragma once

#include "Scene.h"
#include "GameEngine.h"
#include "Action.hpp"

#include <string>
#include <vector>

class Scene_Menu : public Scene
{
    std::string m_title;
    std::vector<std::string> m_menuStrings;
    std::vector<std::string> m_levelPaths;
    sf::Text m_menuText;
    size_t m_selectedMenuIndex = 0;

    void init();

    void onEnd() override;
    void update() override;
    void sRender() override;
    void sDoAction(const Action &action) override;

public:
    Scene_Menu(GameEngine &gameEngine);
};