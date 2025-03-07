// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include <enet/enet.h>

#include "NetEntityManager.hpp"
#include "NetworkData.hpp"

class NetworkManager
{
    ENetHost* m_server; // ENet server or client instance

    NetworkData* m_data; // pointer to data if data received

    NetEntityManager m_netEntityMan;

public:

    NetworkManager();
    ~NetworkManager();

    void update(); // called every frame to process network events
    // void processPosition(ENetPacket* packet);
    // void processVelocity(const EntityID EntityID, const Vec2f& vel);
    NetEntityID createNetEntity();

    void sendData(const NetworkData& data);
};