// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Server
#include "MatchmakingServer.hpp"
#include "LobbyServer.hpp" // added by chat

// C++ standard library
#include <thread> // added by chat
#include <vector>

class ServerEngine
{
    MatchmakingServer m_matchmakingServer; // matchmaking thread
    std::vector<std::thread> m_lobbyThreads; // added by chat
    bool m_isRunning = true;
    // void update(); // removed by chat
    void runLobby(LobbyServer& lobby); // added by chat

public:
    ServerEngine();
    ~ServerEngine();

    void run();
};
