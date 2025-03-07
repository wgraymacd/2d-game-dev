// Copyright 2025, William MacDonald, All Rights Reserved.

#include "ServerEngine.hpp"

/// @brief continuously calls ServerEngine::update while the game is still running
void ServerEngine::run()
{
    while (true)
    {
        update();
    }
}

/// @brief updates the game state with ServerEngine::sUserInput and the active scene's update function
void ServerEngine::update()
{
    m_netManager.update();
}
