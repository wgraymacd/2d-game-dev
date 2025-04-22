// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Physics
#include "physics/Vec2.hpp"

// Utility
#include "utility/ClientGlobals.hpp"

// Global
#include "NetworkData.hpp"

// C++ standard libraries
#include <enet/enet.h>
#include <unordered_map>
#include <vector>

class NetworkManager
{
    ENetHost* m_client;
    ENetPeer* m_peer;

    NetworkData* m_data; // pointer to single data element received if received, else nullptr
    std::vector<NetworkData> m_dataVec;

    std::unordered_map<EntityID, EntityID> m_netToLocalID; // map[net] = local
    std::unordered_map<EntityID, EntityID> m_localToNetID; // map[local] = net

public:

    NetworkManager();
    ~NetworkManager();

    void update(); // called every frame to process network events

    /// @brief get data received from server
    const std::vector<NetworkData>& getData() const;

    /// TODO: test this with different OSs, check for endianness, same floating-point rep, padding
    /// @brief send data to server, only works with POD
    void sendData(const NetworkData& data) const;

    void updateIDMaps(EntityID localID, EntityID netID);

    EntityID getLocalID(EntityID netID) const;
    EntityID getNetID(EntityID localID) const;

    void connectTo(int addressP1, int addressP2, int addressP3, int addressP4, int port);
    void disconnect();
};
