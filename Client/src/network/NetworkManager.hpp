// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Physics
#include "physics/Vec2.hpp"

// Utility
#include "utility/ClientGlobals.hpp"

// Global
#include "NetworkDatum.hpp"

// C++ standard libraries
#include <enet/enet.h>
#include <array>
#include <vector>

class NetworkManager
{
    ENetHost* m_client = nullptr;
    ENetPeer* m_peer = nullptr;

    std::vector<NetworkDatum> m_dataVec;

    /// @todo could make unordered_map instead, removal of IDs fast with map.erase(key) function
    std::array<EntityID, Settings::worldMaxEntities> m_netToLocalID; // map[net] = local
    std::array<EntityID, Settings::worldMaxEntities> m_localToNetID; // map[local] = net

public:

    NetworkManager();
    ~NetworkManager();

    void update(); // called every frame to process network events

    /// @brief get data received from server
    const std::vector<NetworkDatum>& getData() const;

    /// TODO: test this with different OSs, check for endianness, same floating-point rep, padding
    /// @brief send data to server, only works with POD
    void sendData(const NetworkDatum& data) const;

    void updateIDMaps(EntityID localID, EntityID netID);

    EntityID getLocalID(EntityID netID) const;
    EntityID getNetID(EntityID localID) const;

    void connectTo(int addressP1, int addressP2, int addressP3, int addressP4, int port);
    void disconnect();
};
