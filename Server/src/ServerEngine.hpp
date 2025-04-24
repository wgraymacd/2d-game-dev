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
public:

    ServerEngine();
    ~ServerEngine();

    void run();

private:

    MatchmakingServer m_matchmakingServer;
    std::vector<std::thread> m_lobbyThreads;
    bool m_isRunning = true;

    void runMatchmaking();
};
