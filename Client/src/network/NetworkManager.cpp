// Copyright 2025, William MacDonald, All Rights Reserved.

/// PLAN:
/*
server doesn't need graphics, player input, UI, etc. and players don't need AI, physics, game rules, etc.
however, if I ever want a private match or something, then need ability to run your own server

client sends data to server like player pos
server broadcasts data to all clients for clients to use

1.
data is just render-ready info like pos, rotation, etc.

2.
data is client input and server processes that data and returns render-ready info

client should run render, user input, player movement, bullet spawns, player collision
the rest is choice, should do tests for speed and such to see if it makes sense to run bullet firing for e.g. locally or via the server

*/

#include "NetworkManager.hpp"

#include <iostream>
#include <vector>

/// @brief initializes enet and creates client + connection
NetworkManager::NetworkManager() {
    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet.\n";
        exit(EXIT_FAILURE);
    }

    // create client
    m_client = enet_host_create(nullptr, 1, 2, 0, 0); // no specified address, so client, can handle only 1 peer (the server), 2 communication channels, and no bandwidth limits
    if (!m_client) {
        std::cerr << "Failed to create client.\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Client host created: host->address.host = " << m_client->address.host << " and host->address.port = " << m_client->address.port << ".\n";

    // connect to server 127.0.0.1, localhost
    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 54000;
    std::cout << "Defined server 127.0.0.1: address.host = " << address.host << " and address.port = " << address.port << ".\n";

    // send connection request to server, allocating two channels 0 and 1 (peer: connection in network, can be either a client connected to a server or a server that the client is connected to)
    m_peer = enet_host_connect(m_client, &address, 2, 0); // client's host instance, server's address, channels, no user data passed to connection
    if (!m_peer) {
        std::cerr << "Failed to connect to server.\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Initialized connection: peer->address.host = " << m_peer->address.host << " and peer->address.port = " << m_peer->address.port << ".\n";

    // wait for connection to be established to ensure peer is fully initialized, sync mechanism
    ENetEvent event;
    if (enet_host_service(m_client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connection to server established.\n";
    }
    else {
        std::cerr << "Connection to server failed.\n";
        enet_peer_reset(m_peer);
        exit(EXIT_FAILURE);
    }
}

/// @brief properly cleans up network resources
NetworkManager::~NetworkManager() {
    enet_host_destroy(m_client);
    enet_deinitialize();
}

/// @brief processes network events for the server and clients, checks for incoming connections, received messages, and disconnections
void NetworkManager::update() {
    ENetEvent event;

    m_dataVec.clear(); // clear data from last call to update(), ensure that data is processed before next call to update() or data is overwritten

    while (enet_host_service(m_client, &event, 0) > 0) // enet_host_service polls for network events and sends queued packets (&event: stores the next network event if one exists; 0: doesn't wait for new events)
    {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            std::cout << "Connected to server.\n";
            // store client info here if needed with event.peer->data
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            m_data = (NetworkData*)event.packet->data;
            m_dataVec.push_back(*m_data);

            // switch (m_data->dataType) {
            // case POSITION:
            //     std::cout << "Received: " << std::to_string(m_data->dataType) << ", " << m_data->netID << ", " << m_data->data.floatVec.x << ", " << m_data->data.floatVec.y << "\n";
            //     break;
            // case VELOCITY:
            //     std::cout << "Received: " << std::to_string(m_data->dataType) << ", " << m_data->netID << ", " << m_data->data.floatVec.x << ", " << m_data->data.floatVec.y << "\n";
            //     break;
            // case SPAWN:
            //     std::cout << "Received: " << std::to_string(m_data->dataType) << ", " << m_data->netID << ", " << m_data->data.floatVec.x << ", " << m_data->data.floatVec.y << "\n";
            //     break;
            // }

            // things received:
            /*
            everything that must be the same for all players:
            player positions and aiming directions (and vel and all that if doing local calculation corrected by server simulation)
            ragdoll positions (if they are interactable or explosive or anything that affects gameplay)
            bullet spawn positions (and maybe even updated positions as traveling, but could update locally)
            */

            // what to do with them:
            /*
            update entity components in entity map accordingly so that entity systems operate on received data
            */

            enet_packet_destroy(event.packet); // clean up memory after processing message
            m_data = nullptr;
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "Disconnected from server.\n";
            // reset peer's client info here if stored above with event.peer->data = nullptr;
            break;
        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }

    /// TODO: one way to make changes with the received data is to return it and get it to ScenePlay.cpp
    /// can create a vector of NetworkData objects (each one appended in an iteration of the while loop) and perform all logic with data in ScenePlay.cpp
    /// other method is to change things here directly before entering the ScenePlay.cpp file
    /// would have to include Entity, Components, etc., making this library now dependent on the core library (could pull Entity and Components out of core though)
}

const std::vector<NetworkData>& NetworkManager::getData() const {
    return m_dataVec;
}

void NetworkManager::sendData(const NetworkData& data) const {
    if (!m_client) {
        std::cerr << "Client host is not initialized.\n";
        return;
    }
    if (!m_peer || m_peer->state != ENET_PEER_STATE_CONNECTED) {
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
        sizeof(NetworkData),
        ENET_PACKET_FLAG_RELIABLE
    ); // reliable means guaranteed to be delivered

    if (!packet) {
        std::cerr << "Packet creation failed.\n";
        return;
    }

    // send to server
    if (enet_peer_send(m_peer, 0, packet) < 0) // packet queued but not immediately transmitted
    {
        std::cerr << "Failed to send packet.\n";
        return;
    }

    // send out queued packets without dispatching any events
    enet_host_flush(m_client); // forces immediate packet transmition, no wait for enet_host_service, just gives control over send timing really
}

void NetworkManager::updateIDMaps(EntityID netID, EntityID localID) {
    m_netToLocalID[netID] = localID;
    m_localToNetID[localID] = netID;
}

EntityID NetworkManager::getLocalID(EntityID netID) const {
    return m_netToLocalID.at(netID);
}

EntityID NetworkManager::getNetID(EntityID localID) const {
    return m_localToNetID.at(localID);
}
