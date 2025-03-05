// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include <enet/enet.h>
#include <iostream>

class NetworkManager
{
    ENetHost* server; // ENet server or client instance

public:

    NetworkManager();
    ~NetworkManager();

    void update(); // called every frame to process network events
    void sendMessage(const std::string& message);
};