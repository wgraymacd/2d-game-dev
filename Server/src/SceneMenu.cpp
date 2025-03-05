// Copyright 2025, William MacDonald, All Rights Reserved.

#include "SceneMenu.hpp"
#include "ScenePlay.hpp"
#include "Scene.hpp"
#include "Assets.hpp"
#include "GameEngine.hpp"
#include "Action.hpp"

/// @brief constructs a new SceneMenu object, calls SceneMenu::init
/// @param gameEngine the game's main engine; required by Scene to access the GameEngine object
SceneMenu::SceneMenu(GameEngine& gameEngine)
    : Scene(gameEngine)
{
    init();
}

/// @brief initialized the MENU scene: registers keybinds, sets text attributes, and defines level paths
void SceneMenu::init()
{

}

/// @brief updates the scene's state
void SceneMenu::updateState(std::chrono::duration<long long, std::nano>& lag)
{
    /// TODO: implement lag catching up
    m_game.getNetManager().update();
}

/// @brief performs the given action
/// @param action an Action to perform; action has a type and a name
void SceneMenu::sDoAction(const Action& action)
{
    if (action.type() == START)
    {
        if (action.name() == "QUIT")
        {
            onEnd();
        }
    }
}

/// @brief quits the game
void SceneMenu::onEnd()
{
    m_game.quit();
}