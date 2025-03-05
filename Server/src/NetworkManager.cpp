// Copyright 2025, William MacDonald, All Rights Reserved.

/// TODO: checking for if (isServer) for now is fine, but ultimately may want to either create two classes Client and Server with high-level functions that handle related logic to separate the two, or create two entirely separate codebases, one for the server and one for the players to download (max efficiency since server doesn't need graphics, player input, UI, etc. and players don't need AI, physics, game rules, etc.); however, if I ever want a private match or something, then need ability to run your own server

#include "NetworkManager.hpp"

#include <iostream>

/// @brief initializes enet and creates server
NetworkManager::NetworkManager()
{
    if (enet_initialize() != 0)
    {
        std::cerr << "Failed to initialize ENet.\n";
        exit(EXIT_FAILURE);
    }

    // creat server
    ENetAddress address;
    address.host = ENET_HOST_ANY; // accept connections from any IP address (host: device connected to a network, both server and client are hosts)
    address.port = 54000; // listen for incoming connections on this port (port: number that IDs a specific process or service running on a host, clients must connects to this port)
    server = enet_host_create(&address, 32, 2, 0, 0); // 32 clients and/or outgoing connections, 2 channels used (0 and 1, e.g., one for game state and one for chat), any amount of incoming bandwidth, any amount of outgoing bandwidth
    if (!server)
    {
        std::cerr << "Failed to create server.\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Server started: address.host = " << address.host << " and address.port = " << address.port << ".\n";

}

/// @brief properly cleans up network resources
NetworkManager::~NetworkManager()
{
    enet_host_destroy(server);
    enet_deinitialize();
}

/// @brief processes network events for the server and clients, checks for incoming connections, received messages, and disconnections
void NetworkManager::update()
{
    ENetEvent event;
    enet_uint32 waitTime = 0; // ms, time to pause program until event received or time runs out (-1 for indefinite pause), will skip over events if they occur between checks, lower timout = more frequent calls to this function
    // enet_host_service checks for pending network events in a short time window and returns 0 if no events found during that window
    // polls events from an internal event queue maintained by ENet, network events are added to this queue
    // if queue empty and 0 timeout, immediately returns
    // if one event missed, then upon receiving next, both are processed
    while (enet_host_service(server, &event, waitTime) > 0) // enet_host_service polls for network events (&event: stores the next network event if one exists; 0: doesn't wait for new events)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT: /// TODO: this event is not processed until the first message is sent from the client, could be a localhost thing, idk, but look into it later
            std::cout << "Client connected (host = "
                << event.peer->address.host
                << ", port = " << event.peer->address.port << ").\n";
            // store client info here if needed with event.peer->data
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            std::cout << "Received: " << (char*)event.packet->data << "\n"; // data can be of any type needed, event.packet->data is a raw pointer to binary data


            // things received:
            /*
            everything that other clients need from any given client:
            player position and aiming direction (and vel and all that if doing local calculation corrected by server simulation)
            ragdoll spawn initial conditions (if not just entirely run by server)
            bullet spawn position (and maybe even updated positions as traveling, or vel too and run on server, but could update locally)
            */

            // what to do with them:
            /*
            broadcast to all clients
            */


            enet_packet_destroy(event.packet); // clean up memory after processing message
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "Client disconnected.\n";
            // reset peer's client info here if stored above with event.peer->data = nullptr;
            break;
        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}

void NetworkManager::sendMessage(const std::string& message)
{
    ENetPacket* packet = enet_packet_create(message.c_str(), message.size() + 1, ENET_PACKET_FLAG_RELIABLE); // reliable means guaranteed to be delivered

    // send to all clients
    enet_host_broadcast(server, 0, packet);

    // send out queued packets without dispatching any events
    enet_host_flush(server);
}