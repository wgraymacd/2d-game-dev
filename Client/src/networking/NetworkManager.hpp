// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include <enet/enet.h>
#include <iostream>
#include <vector>

#include "NetworkSerializer.hpp"

class NetworkManager
{
    ENetHost* client;
    ENetPeer* peer;

public:

    NetworkManager();
    ~NetworkManager();

    void update(); // called every frame to process network events

    /// @brief send data to server, can be POD or vectors of POD types
    template <typename T>
    void sendData(const T& data)
    {
        if (!client) {
            std::cerr << "Client host is not initialized.\n";
            return;
        }
        if (!peer || peer->state != ENET_PEER_STATE_CONNECTED) {
            std::cerr << "No peer connection established.\n";
            return;
        }

        // std::vector<uint8_t> serializedData = NetworkSerializer<T>::serialize(data);
        // ENetPacket* packet = enet_packet_create(
        //     serializedData.data()
        //     serializedData.size(),
        //     ENET_PACKET_FLAG_RELIABLE
        // ); // reliable means guaranteed to be delivered

        ENetPacket* packet = enet_packet_create(
            &data,
            sizeof(T),
            ENET_PACKET_FLAG_RELIABLE
        ); // reliable means guaranteed to be delivered

        if (!packet)
        {
            std::cerr << "Packet creation failed.\n";
            return;
        }

        // send to server
        if (enet_peer_send(peer, 0, packet) < 0)
        {
            std::cerr << "Failed to send packet.\n";
            return;
        }

        // send out queued packets without dispatching any events
        enet_host_flush(client);
    }
};