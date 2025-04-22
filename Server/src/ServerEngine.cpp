// Copyright 2025, William MacDonald, All Rights Reserved.

// Server
#include "ServerEngine.hpp"

// External libraries
#include <enet/enet.h>

// C++ standard library
#include <iostream>
#include <vector>
#include <thread> // added by chat

ServerEngine::ServerEngine()
{
    if (enet_initialize() != 0)
    {
        std::cerr << "Failed to initialize ENet" << std::endl;
        exit(1);
    }
}

ServerEngine::~ServerEngine()
{
    enet_deinitialize();
}

// before chat
// void ServerEngine::run() {
//     while (m_isRunning) {
//         update();
//     }
// }

// added by chat
void ServerEngine::run()
{
    // Start the matchmaking server in the main thread
    std::thread matchmakingThread([this]()
        {
            while (m_isRunning)
            {
                m_matchmakingServer.update();
            }
        });

    // Create and start threads for each lobby server
    for (LobbyServer& lobby : m_matchmakingServer.getActiveLobbies())
    {
        m_lobbyThreads.emplace_back(&ServerEngine::runLobby, this, std::ref(lobby));
    }

    // Wait for the matchmaking thread to finish
    matchmakingThread.join();

    // Wait for all lobby threads to finish
    for (std::thread& thread : m_lobbyThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

// removed by chat
// /// @brief updates the server state
// void ServerEngine::update() {
//     // connect clients to lobbies and runs lobbies
//     m_matchmakingServer.update();
// }

// added by chat
void ServerEngine::runLobby(LobbyServer& lobby)
{
    while (m_isRunning)
    {
        lobby.update();
    }
}
