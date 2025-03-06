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

    template <typename T>
    void sendMessage(const T& message)
    {
        ENetPacket* packet = enet_packet_create(message.c_str(), message.size() + 1, ENET_PACKET_FLAG_RELIABLE); // reliable means guaranteed to be delivered

        // send to all clients
        enet_host_broadcast(server, 0, packet);

        // send out queued packets without dispatching any events
        enet_host_flush(server);
    }
};