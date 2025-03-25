// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "NetEntityManager.hpp"
#include "NetworkData.hpp"

#include <enet/enet.h>

class NetworkManager {
    ENetHost* m_server; // ENet server or client instance

    NetworkData* m_data; // pointer to data if data received

    NetEntityManager m_netEntityMan;

public:

    NetworkManager();
    ~NetworkManager();

    void update(); // called every frame (TODO: change to time, not frames) to process network events
    // void processPosition(ENetPacket* packet);
    // void processVelocity(const EntityID EntityID, const Vec2f& vel);

    EntityID createNetEntity();

    /// @brief send data to all connected clients
    void broadcastData(const NetworkData& data);

    /// @brief send data to a single client
    void sendData(ENetPeer* clientPeer, const NetworkData& data);
};
