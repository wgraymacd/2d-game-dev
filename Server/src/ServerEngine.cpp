// Copyright 2025, William MacDonald, All Rights Reserved.

#include "ServerEngine.hpp"

/// @brief continuously calls ServerEngine::update while the game is still running
void ServerEngine::run() {
    /// TODO: update to while is running
    while (m_running) {
        update();
    }
}

/// @brief updates the server state 
void ServerEngine::update() {
    m_netManager.update();
}
