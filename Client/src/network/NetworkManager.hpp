// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include <enet/enet.h>
#include <unordered_map>

#include "utility/Globals.hpp"
#include "physics/Vec2.hpp"
#include "NetworkData.hpp"

// #include "NetworkSerializer.hpp"

class NetworkManager
{
    ENetHost* m_client;
    ENetPeer* m_peer;

    NetworkData* m_data = nullptr; // pointer to data received if received, else nullptr

    std::unordered_map<EntityID, EntityID> m_mapID; // map[local] = net

public:

    NetworkManager();
    ~NetworkManager();

    void update(); // called every frame to process network events
    void processPosition(const EntityID entityID, const Vec2f& pos);
    void processVelocity(const EntityID entityID, const Vec2f& vel);
    void updateIDMap(const EntityID localID, const EntityID netID);

    /// TODO: test this with different OSs, check for endianness, same floating-point rep, padding
    /// @brief send data to server, only works with POD
    void sendData(const NetworkData& data);

    EntityID getNetID(const EntityID localID);
};