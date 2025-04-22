// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Server
#include "ServerGlobals.hpp"
#include "LobbyEntityManager.hpp"

// Global
#include "Random.hpp"
#include "NetworkData.hpp"

// External libraries
#include <enet/enet.h>

// C++ standard library
#include <vector>
#include <iostream>
#include <limits>

class LobbyServer
{
public:

    LobbyServer(int port) : m_port(port)
    {
        m_clientIDs.reserve(m_maxPlayers);

        std::cout << "World seed initialized to: " << m_worldSeed << "\n";
    }

    void update()
    {
        ENetEvent event;
        enet_uint32 waitTime = 0; // ms, time to pause program until event received or time runs out (-1 for indefinite pause), will skip over events if they occur between checks, lower timout = more frequent calls to this function
        // enet_host_service checks for pending network events in a short time window and returns 0 if no events found during that window
        // polls events from an internal event queue maintained by ENet, network events are added to this queue
        // if queue empty and 0 timeout, immediately returns
        // if one event missed, then upon receiving next, both are processed
        while (enet_host_service(m_server, &event, waitTime) > 0) // enet_host_service polls for network events (&event: stores the next network event if one exists; 0: doesn't wait for new events)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT: /// @todo this event is not processed until the first message is sent from the client, could be a localhost thing, idk, but look into it later
                    std::cout << "Client connected (host = " << event.peer->address.host << ", port = " << event.peer->address.port << ")\n";
                    // store client info here if needed with event.peer->data

                    std::cout << "Sending world seed\n";

                    sendData(event.peer, NetworkData { NetworkData::DataType::WORLD_SEED, { .i = m_worldSeed } });

                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                {
                    NetworkData* received = (NetworkData*)event.packet->data;

                    switch (received->dataType)
                    {
                        std::cout << received << "\n";

                        case NetworkData::DataType::POSITION:
                            enet_host_broadcast(m_server, 0, event.packet);
                            enet_host_flush(m_server); // automatically handles memory cleanup, cannot use with enet_packet_destroy
                            break;

                        case NetworkData::DataType::VELOCITY:
                            enet_host_broadcast(m_server, 0, event.packet);
                            enet_host_flush(m_server);
                            break;

                        case NetworkData::DataType::SPAWN:
                        {
                            float tempStorage;

                            tempStorage = received->second.f;

                            // send out LOCAL_SPAWN to specific client
                            received->dataType = NetworkData::DataType::LOCAL_SPAWN;
                            received->second.id = createNetEntity();
                            sendData(event.peer, *received);

                            // send SPAWN to everyone
                            received->dataType = NetworkData::DataType::SPAWN;
                            received->first.id = received->second.id;
                            received->second.f = tempStorage;
                            broadcastData(*received);

                            // clean up memory after processing message, careful not to clean memory in use or already cleaned memory cuz will seg fault
                            enet_packet_destroy(event.packet);

                            break;
                        }

                        case NetworkData::DataType::NONE:
                        case NetworkData::DataType::LOBBY_CONNECT:
                        case NetworkData::DataType::LOCAL_SPAWN:
                        case NetworkData::DataType::WORLD_SEED:
                        case NetworkData::DataType::NUM_TYPES:
                            break;
                    }

                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Client disconnected\n";
                    // reset peer's client info here if stored above with event.peer->data = nullptr;
                    break;

                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }
    }

    bool isFull() const
    {
        return m_clientIDs.size() >= m_maxPlayers;
    }

    int getPort() const
    {
        return m_port;
    }

private:

    ENetHost* m_server;
    int m_port;

    LobbyEntityManager m_lobbyEntityMan;
    std::vector<EntityID> m_clientIDs;
    const EntityID m_maxPlayers { Settings::maxLobbyPlayers };

    const int m_worldSeed { Random::getIntegral(0, std::numeric_limits<int>::max()) };

    EntityID createNetEntity()
    {
        return m_lobbyEntityMan.addNetEntity();
    }

    void broadcastData(const NetworkData& data)
    {
        std::cout << "Broadcasting data\n";

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

        // queue packet for broadcast
        enet_host_broadcast(m_server, 0, packet);

        // send out queued packets without dispatching any events
        enet_host_flush(m_server);
    }

    void sendData(ENetPeer* clientPeer, const NetworkData& data)
    {
        std::cout << "Sending data to client\n";

        ENetPacket* packet = enet_packet_create(
            &data,
            sizeof(NetworkData),
            ENET_PACKET_FLAG_RELIABLE
        );

        if (!packet)
        {
            std::cerr << "Packet creation failed\n";
            return;
        }

        enet_peer_send(clientPeer, 0, packet); // Send to a single client
        enet_host_flush(m_server);
    }
};
