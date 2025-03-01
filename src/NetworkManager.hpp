// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include <enet/enet.h>
#include <iostream>

class NetworkManager
{
    ENetHost* host; // ENet server or client instance
    ENetPeer* peer; // only used by clients
    bool isServer;

public:

    NetworkManager(bool isServer);
    ~NetworkManager();

    void update(); // called every frame to process network events
    void sendMessage(const std::string& message);
};