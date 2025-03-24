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

    /// @brief creates a new net entity and returns its net ID
    EntityID createNetEntity();

    void broadcastData(const NetworkData& data);

    /// @brief send a single client the id of the client's entity spawned, for use with DataType::SPAWN
    void sendNetID(ENetPeer* clientPeer, const NetworkData& data);
};
