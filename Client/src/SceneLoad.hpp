// Copyright 2025, William MacDonald, All Rights Reserved.

// #pragma once

// #include <SFML/Graphics.hpp>

// #include "Scene.hpp"
// #include "GameEngine.hpp"
// #include "Globals.hpp"

// class SceneLoad : public Scene
// {
//     const Vec2ui m_worldMaxCells = GlobalSettings::worldMaxCells;
//     const unsinged int m_cellSizePixels = GlobalSettings::cellSizePixels;

//     std::string m_loadStep;
//     sf::Text m_loadText = sf::Text(m_game.assets().getFont("font"));

//     void init();
//     void onEnd() override;
//     void update(std::chrono::duration<long long, std::nano>& lag) override; /// TODO: this class does not need the lag param, fix this
//     void sRender() override;

//     void loadGame(); /// TODO: may add param here to differentiate between game types or something
//     void generateWorld();

// public:

//     SceneLoad(GameEngine& gameEngine);
// };