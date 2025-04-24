// Copyright 2025, William MacDonald, All Rights Reserved.

// Server
#include "ServerEngine.hpp"

// External libraries
#include <enet/enet.h>

// C++ standard library
#include <iostream>
#include <vector>
#include <thread>

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

void ServerEngine::run()
{
    // Start the matchmaking server in the main thread
    /// todo@ could make it so that runMatchmaking is a global static function or whatever and not have to use the method version and this and all all that if orginzation is better
    std::thread matchmakingThread(&ServerEngine::runMatchmaking, this);

    // Create and start threads for each lobby server
    /// @todo the problem is that the lobbies are not yet created when this function is called, so the threads are not started
    /// @todo need to create a lobby thread for each active lobby server in the matchmaking server
    // for (LobbyServer& lobby : m_matchmakingServer.getActiveLobbies())
    // {
    //     m_lobbyThreads.emplace_back(&ServerEngine::runLobby, this, std::ref(lobby));
    // }

    // Wait for the matchmaking thread to finish
    matchmakingThread.join();


    /// @todo this will go somewhere else for now, but if for any reason I want a lobby to be active while the matchmaking server is not, I can have the lobby logic and all that happen here instead of being owned by the matchmaking server
    // Wait for all lobby threads to finish
    // for (std::thread& thread : m_lobbyThreads)
    // {
    //     if (thread.joinable())
    //     {
    //         thread.join();
    //     }
    // }
}

/// @todo add logic for m_isRunning, make sure matchmaking thread (which is main thread I think) finishes properly (after a whole call to update)
void ServerEngine::runMatchmaking()
{
    while (m_isRunning)
    {
        m_matchmakingServer.update();
    }
}

/// @todo use if implementing logic here, but logic in matchmaking server now
// void ServerEngine::runLobby(LobbyServer& lobby)
// {
//     while (m_isRunning)
//     {
//         lobby.update();
//     }
// }
