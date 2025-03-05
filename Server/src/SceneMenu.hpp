// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "Scene.hpp"
#include "GameEngine.hpp"
#include "Action.hpp"

#include <string>
#include <vector>
#include <chrono>

class SceneMenu : public Scene
{
    void init();

    void onEnd() override;
    void updateState(std::chrono::duration<long long, std::nano>& lag) override;
    void sDoAction(const Action& action) override;

public:
    SceneMenu(GameEngine& gameEngine);
};