#include "NetworkManager.hpp"

#include <iostream>

NetworkManager::NetworkManager(bool isServer)
{
    if (enet_initialize() != 0)
    {
        std::cerr << "Failed to initialize ENet.\n";
        exit(EXIT_FAILURE);
    }

    if (isServer)
    {
        // create server
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = 54000;
        host = enet_host_create(&address, 32, 2, 0, 0); // 32 clients and/or outgoing connections, 2 channels used (0 and 1), any amount of incoming bandwidth, any amount of outgoing bandwidth
        if (!host)
        {
            std::cerr << "Failed to create server.\n";
            exit(EXIT_FAILURE);
        }
        std::cout << "Server started.\n";
    }
    else // client
    {
        // create client
        host = enet_host_create(nullptr, 1, 2, 0, 0); // no specified address, so client
        if (!host)
        {
            std::cerr << "Failed to create client.\n";
            exit(EXIT_FAILURE);
        }

        // connect to server 127.0.0.1
        ENetAddress address;
        enet_address_set_host(&address, "127.0.0.1");
        address.port = 54000;

        // init connection, allocating two channels 0 and 1
        peer = enet_host_connect(host, &address, 2, 0);
        if (!peer)
        {
            std::cerr << "Failed to connect to server.\n";
            exit(EXIT_FAILURE);
        }
    }
}

NetworkManager::~NetworkManager()
{
    enet_host_destroy(host);
    enet_deinitialize();
}

void NetworkManager::update()
{
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            std::cout << (isServer ? "Client connected.\n" : "Connected to server.\n");
            // store relevant client info here if needed
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            std::cout << "Received: " << (char*)event.packet->data << "\n";
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << (isServer ? "Client disconnected.\n" : "Disconnected from server.\n");
            // reset peer's client info here if stored above with event.peer->data = nullptr;
            break;
        }
    }
}

void NetworkManager::sendMessage(const std::string& message)
{
    ENetPacket* packet = enet_packet_create(message.c_str(), message.size() + 1, ENET_PACKET_FLAG_RELIABLE); // reliable means guaranteed to be delivered

    if (isServer)
    {
        // send to all clients
        enet_host_broadcast(host, 0, packet);
    }
    else
    {
        // send to server
        enet_peer_send(peer, 0, packet);
    }

    // send out queued packets without dispatching any events
    enet_host_flush(host);
}