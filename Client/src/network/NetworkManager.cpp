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

// Network
#include "NetworkManager.hpp"

// C++ standard libraries
#include <iostream>
#include <vector>
#include <string>

/// @brief initializes enet and creates client + connection
NetworkManager::NetworkManager()
{
    if (enet_initialize() != 0)
    {
        std::cerr << "Failed to initialize ENet\n";
        exit(EXIT_FAILURE);
    }

    // create client
    m_client = enet_host_create(nullptr, 1, 2, 0, 0); // no specified address, so client, can handle only 1 peer (the server), 2 communication channels, and no bandwidth limits
    if (!m_client)
    {
        std::cerr << "Failed to create client\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Client host created: " << m_client->address.host << ":" << m_client->address.port << "\n";

    // connect to server 127.0.0.1, localhost, on port 5000
    connectTo(127, 0, 0, 1, 5000);
}

/// @brief properly cleans up network resources
NetworkManager::~NetworkManager()
{
    enet_host_destroy(m_client);
    enet_deinitialize();
}

/// @brief processes network events for the server and clients, checks for incoming connections, received messages, and disconnections
void NetworkManager::update()
{
    ENetEvent event;

    m_dataVec.clear(); // clear data from last call to update(), ensure that data is processed before next call to update() or data is overwritten

    while (enet_host_service(m_client, &event, 0) > 0) // enet_host_service polls for network events and sends queued packets (&event: stores the next network event if one exists; 0: doesn't wait for new events)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "Connected to " << m_peer->address.host << ":" << m_peer->address.port << "\n";
                // store client info here if needed with event.peer->data
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                m_data = (NetworkData*)event.packet->data;
                m_dataVec.push_back(*m_data); /// TODO: consider adding things to multiple vectors, one for each type of data so that I can access each one at separate times in ScenePlay.cpp

                enet_packet_destroy(event.packet); // clean up memory after processing message
                m_data = nullptr;
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "Disconnected from " << m_peer->address.host << ":" << m_peer->address.port << "\n";
                // reset peer's client info here if stored above with event.peer->data = nullptr;
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
        }
    }
}

const std::vector<NetworkData>& NetworkManager::getData() const
{
    return m_dataVec;
}

void NetworkManager::sendData(const NetworkData& data) const
{
    std::cout << "Sending: " << std::to_string(data.dataType) << "\n";

    if (!m_client)
    {
        std::cerr << "Client host is not initialized\n";
        return;
    }
    if (!m_peer || m_peer->state != ENET_PEER_STATE_CONNECTED)
    {
        std::cerr << "No peer connection established\n";
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

    if (!packet)
    {
        std::cerr << "Packet creation failed\n";
        return;
    }

    // send to server
    if (enet_peer_send(m_peer, 0, packet) < 0) // packet queued but not immediately transmitted
    {
        std::cerr << "Failed to send packet\n";
        return;
    }

    // send out queued packets without dispatching any events
    enet_host_flush(m_client); // forces immediate packet transmition, no wait for enet_host_service, just gives control over send timing really
}

void NetworkManager::updateIDMaps(EntityID localID, EntityID netID)
{
    std::cout << "Mapping localID " << localID << " to netID " << netID << "\n";
    m_netToLocalID[netID] = localID;
    m_localToNetID[localID] = netID;
}

EntityID NetworkManager::getLocalID(EntityID netID) const
{
    return m_netToLocalID.at(netID);
}

EntityID NetworkManager::getNetID(EntityID localID) const
{
    return m_localToNetID.at(localID);
}

void NetworkManager::connectTo(int addressP1, int addressP2, int addressP3, int addressP4, int port)
{
    // ensure any existing connections are terminated
    disconnect();

    const std::string addressString =
        std::to_string(addressP1) + "." +
        std::to_string(addressP2) + "." +
        std::to_string(addressP3) + "." +
        std::to_string(addressP4);

    ENetAddress address;
    enet_address_set_host(&address, addressString.c_str());
    address.port = static_cast<enet_uint16>(port);

    // send connection request to server, allocating two channels 0 and 1 (peer: connection in network, can be either a client connected to a server or a server that the client is connected to)
    m_peer = enet_host_connect(m_client, &address, 2, 0); // client's host instance, server's address, channels, no user data passed to connection
    if (!m_peer)
    {
        std::cerr << "Failed to initiate connection\n";
        exit(1);
    }
    std::cout << "Initialized connection to " << m_peer->address.host << ":" << m_peer->address.port << "\n";

    // wait for connection to be established to ensure peer is fully initialized, sync mechanism
    ENetEvent event;
    if (enet_host_service(m_client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        std::cout << "Connection to " << address.host << ":" << address.port << " succeeded\n";
    }
    else
    {
        std::cerr << "Connection to " << address.host << ":" << address.port << " failed\n";
        enet_peer_reset(m_peer);
        exit(1);
    }
}

void NetworkManager::disconnect()
{
    if (m_peer)
    {
        enet_peer_disconnect(m_peer, 0);
        ENetEvent event;
        while (enet_host_service(m_client, &event, 3000) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Disconnected from " << m_peer->address.host << ":" << m_peer->address.port << "\n";
                    m_peer = nullptr;
                    return;

                default:
                    break;
            }
        }
        enet_peer_reset(m_peer);
        m_peer = nullptr;
    }
}
