// Copyright 2025, William MacDonald, All Rights Reserved.

#include "NetworkManager.hpp"

#include <iostream>

/// @brief initializes enet and creates server
NetworkManager::NetworkManager() {
    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet.\n";
        exit(EXIT_FAILURE);
    }

    // create server
    ENetAddress address;
    address.host = ENET_HOST_ANY; // accept connections from any IP address (host: device connected to a network, both server and client are hosts)
    address.port = 54000; // listen for incoming connections on this port (port: number that IDs a specific process or service running on a host, clients must connects to this port)
    m_server = enet_host_create(&address, 32, 2, 0, 0); // 32 clients and/or outgoing connections, 2 channels used (0 and 1, e.g., one for game state and one for chat), any amount of incoming bandwidth, any amount of outgoing bandwidth
    if (!m_server) {
        std::cerr << "Failed to create server.\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Server started: address.host = " << address.host << " and address.port = " << address.port << ".\n";
    /// TODO: put this output stuff into a log file instead of console, then can have simple message like "Stop server? (Y/n)" 

}

/// @brief properly cleans up network resources as per enet documentation
NetworkManager::~NetworkManager() {
    enet_host_destroy(m_server);
    enet_deinitialize();
}

/// @brief processes network events for the server and clients, checks for incoming connections, received messages, and disconnections
void NetworkManager::update() {
    ENetEvent event;
    enet_uint32 waitTime = 0; // ms, time to pause program until event received or time runs out (-1 for indefinite pause), will skip over events if they occur between checks, lower timout = more frequent calls to this function
    // enet_host_service checks for pending network events in a short time window and returns 0 if no events found during that window
    // polls events from an internal event queue maintained by ENet, network events are added to this queue
    // if queue empty and 0 timeout, immediately returns
    // if one event missed, then upon receiving next, both are processed
    while (enet_host_service(m_server, &event, waitTime) > 0) // enet_host_service polls for network events (&event: stores the next network event if one exists; 0: doesn't wait for new events)
    {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: /// TODO: this event is not processed until the first message is sent from the client, could be a localhost thing, idk, but look into it later
            std::cout << "Client connected (host = " << event.peer->address.host << ", port = " << event.peer->address.port << ").\n";
            // store client info here if needed with event.peer->data

            // join open lobby or create new one
            // send lobby address to client

            break;

        case ENET_EVENT_TYPE_RECEIVE:
            m_data = (NetworkData*)event.packet->data;

            switch (m_data->dataType) {
            case POSITION:
                std::cout << "Received: " << std::to_string(m_data->dataType) << ", " << m_data->netID << ", " << m_data->data.x << ", " << m_data->data.y << "\n";
                printf("Broadcasting position packet.\n");
                enet_host_broadcast(m_server, 0, event.packet);
                enet_host_flush(m_server); // automatically handles memory cleanup, cannot use with enet_packet_destroy
                break;

            case VELOCITY:
                std::cout << "Received: " << std::to_string(m_data->dataType) << ", " << m_data->netID << ", " << m_data->data.x << ", " << m_data->data.y << "\n";
                printf("Broadcasting velocity packet.\n");
                enet_host_broadcast(m_server, 0, event.packet);
                enet_host_flush(m_server);
                break;

            case SPAWN:
                std::cout << "Received: " << std::to_string(m_data->dataType) << ", " << m_data->localID << ", " << m_data->data.x << ", " << m_data->data.y << "\n";

                std::cout << "Creating new net entity.\n";
                m_data->netID = createNetEntity(); // network ID
                m_data->dataType = LOCAL_SPAWN;
                sendData(event.peer, *m_data);
                m_data->dataType = SPAWN;
                broadcastData(*m_data);
                enet_packet_destroy(event.packet); // clean up memory after processing message, careful not to clean memory in use or already cleaned memory cuz will seg fault
                break;

            case LOBBY_CONNECT:
                std::cout << "Received: " << std::to_string(m_data->dataType) << ", " << m_data->localID << "\n";

                /// @todo
                // find free lobby or create new one
                // send lobby info (address) to client
                // client attempts to connect to lobby
                // lobby creates unique client ID for lobby and sends to client

                break;

            case LOCAL_SPAWN:
            case WORLD_SEED:
                break;
            }


            m_data = nullptr;
            break;

            // things received and sent back:
            /*
            everything that other clients need from any given client:
            player position and aiming direction (and vel and all that if doing local calculation corrected by server simulation)
            ragdoll spawn initial conditions (if not just entirely run by server)
            bullet spawn position (and maybe even updated positions as traveling, or vel too and run on server, but could update locally)
            braodcast all this

            adding entity: add net ID and then send it back to clients to add to net-local ID maps
            */

        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "Client disconnected.\n";
            // reset peer's client info here if stored above with event.peer->data = nullptr;
            break;

        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}

void NetworkManager::broadcastData(const NetworkData& data) {
    std::cout << "Broadcasting data.\n";

    ENetPacket* packet = enet_packet_create(
        &data,
        sizeof(NetworkData),
        ENET_PACKET_FLAG_RELIABLE
    ); // reliable means guaranteed to be delivered

    if (!packet) {
        std::cerr << "Packet creation failed.\n";
        return;
    }

    // queue packet for broadcast
    enet_host_broadcast(m_server, 0, packet);

    // send out queued packets without dispatching any events
    enet_host_flush(m_server);
}

void NetworkManager::sendData(ENetPeer* clientPeer, const NetworkData& data) {
    std::cout << "Sending net ID to client: " << data.netID << ".\n";

    ENetPacket* packet = enet_packet_create(
        &data,
        sizeof(NetworkData),
        ENET_PACKET_FLAG_RELIABLE
    );

    if (!packet) {
        std::cerr << "Packet creation failed.\n";
        return;
    }

    enet_peer_send(clientPeer, 0, packet); // send to a single client
    enet_host_flush(m_server);
}

EntityID NetworkManager::createNetEntity() {
    return m_netEntityMan.addNetEntity();
}
