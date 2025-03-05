// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include <enet/enet.h>
#include <iostream>

class NetworkManager
{
    ENetHost* client;
    ENetPeer* peer;

public:

    NetworkManager();
    ~NetworkManager();

    void update(); // called every frame to process network events
    void sendMessage(const std::string& message);
};