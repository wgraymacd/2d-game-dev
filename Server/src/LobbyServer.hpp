// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Server
#include "ServerGlobals.hpp"
#include "LobbyEntityManager.hpp"

// Global
#include "Random.hpp"
#include "NetworkDatum.hpp"
#include "EntityBase.hpp"

// External libraries
#include <enet/enet.h>

// C++ standard library
#include <vector>
#include <iostream>
#include <limits>
#include <cstring> // for std::memcpy
#include <unordered_map>

/// @todo might need to add mutexes to this class for thread safety, but not sure yet
class LobbyServer
{
public:

    LobbyServer(enet_uint16 port) : m_port(port)
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY; // Accept connections from any IP address
        address.port = m_port;

        m_server = enet_host_create(&address, m_maxPlayers, 2, 0, 0); // Create the ENet server
        if (!m_server)
        {
            std::cerr << "LOBBY: Failed to create ENet server on port " << m_port << "\n";
            exit(1);
        }

        std::cout << "LOBBY: Created, world seed initialized to: " << m_worldSeed << "\n";
    }

    ~LobbyServer()
    {
        if (m_server)
        {
            enet_host_destroy(m_server);
        }

        std::cout << "LOBBY: Server stopped\n";
    }

    void update()
    {
        ENetEvent event;

        while (enet_host_service(m_server, &event, 0) > 0) // enet_host_service polls for network events (&event: stores the next network event if one exists; 0: doesn't wait for new events)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT: /// @todo this event is not processed until the first message is sent from the client, could be a localhost thing, idk, but look into it later
                {
                    std::cout << "LOBBY: Client connected from " << event.peer->address.host << ":" << event.peer->address.port << "\n";
                    // store client info here if needed with event.peer->data

                    ++m_numClients;

                    // Send world seed to the new client
                    sendData(NetworkDatum { NetworkDatum::DataType::WORLD_SEED, .first.i = m_worldSeed }, event.peer);

                    // Send SPAWN data for all existing entities to the new client
                    /// TODO: look into how to batch all this data into a single packet to reduce network overhead
                    const auto& currentState = m_lobbyEntityMan.getCurrentState(); // auto since no access to array size
                    for (EntityID id : m_lobbyEntityMan.getActiveEntities())
                    {
                        sendData(currentState[id], event.peer);
                    }

                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE:
                {
                    NetworkDatum received;
                    std::memcpy(&received, event.packet->data, sizeof(NetworkDatum));
                    std::cout << "LOBBY: Received data: " << received << " from " << event.peer->address.host << ":" << event.peer->address.port << "\n";

                    switch (received.dataType)
                    {
                        case NetworkDatum::DataType::POSITION:
                            broadcastDataExcept(event.packet, event.peer);
                            break;

                        case NetworkDatum::DataType::VELOCITY:
                            broadcastDataExcept(event.packet, event.peer);
                            break;

                        case NetworkDatum::DataType::SPAWN:
                        {
                            EntityID netID = createNetEntity(received);

                            if (received.second.type == EntityBase::Type::PLAYER)
                            {
                                m_client2PlayerID[concatenate(event.peer->address.host, event.peer->address.port)] = netID;

                                received.second.type = EntityBase::Type::ENEMY;
                            }

                            // Send SPAWN to all but specific client
                            NetworkDatum spawn {
                                NetworkDatum::DataType::SPAWN,
                                .first.id = netID,
                                .second.type = received.second.type,
                                .third.f = received.third.f,
                                .fourth.f = received.fourth.f
                            };
                            broadcastDataExcept(spawn, event.peer);

                            // Send LOCAL_SPAWN to specific client
                            NetworkDatum localSpawn {
                                NetworkDatum::DataType::LOCAL_SPAWN,
                                .first.id = received.first.id,
                                .second.id = netID
                            };
                            sendData(localSpawn, event.peer);

                            // Clean up memory after processing message, careful not to clean memory in use or already cleaned memory cuz will seg fault
                            enet_packet_destroy(event.packet);

                            break;
                        }

                        default:
                            break;
                    }

                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT:
                {
                    std::cout << "LOBBY: Client " << event.peer->address.host << ":" << event.peer->address.port << " disconnected\n";

                    unsigned long key = concatenate(event.peer->address.host, event.peer->address.port);

                    // Remove entity from current state
                    m_lobbyEntityMan.destroy(m_client2PlayerID[key]);

                    // Send despawn data to all connected clients
                    NetworkDatum despawn {
                        NetworkDatum::DataType::DESPAWN,
                        .first.id = m_client2PlayerID[key]
                    };
                    broadcastData(despawn);

                    // Erase key-value pair for disconnected client
                    m_client2PlayerID.erase(key);

                    --m_numClients;

                    break;
                }
                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }
    }

    bool isFull() const
    {
        return m_numClients >= m_maxPlayers;
    }

    int getPort() const
    {
        return m_port;
    }

private:

    ENetHost* m_server;
    enet_uint16 m_port;

    LobbyEntityManager m_lobbyEntityMan;
    size_t m_numClients = 0;
    std::unordered_map<unsigned long, EntityID> m_client2PlayerID;
    const size_t m_maxPlayers { Settings::maxLobbyPlayers };

    const int m_worldSeed { Random::getIntegral(0, std::numeric_limits<int>::max()) };

    EntityID createNetEntity(const NetworkDatum& datum)
    {
        return m_lobbyEntityMan.addNetEntity(datum);
    }

    void broadcastData(const NetworkDatum& data)
    {
        std::cout << "LOBBY: Broadcasting data\n";

        ENetPacket* packet = enet_packet_create(
            &data,
            sizeof(NetworkDatum),
            ENET_PACKET_FLAG_RELIABLE
        ); // reliable means guaranteed to be delivered

        if (!packet)
        {
            std::cerr << "LOBBY: Packet creation failed\n";
            return;
        }

        // queue packet for broadcast
        enet_host_broadcast(m_server, 0, packet);

        // send out queued packets without dispatching any events
        enet_host_flush(m_server);
    }

    void broadcastDataExcept(const NetworkDatum& data, ENetPeer* excludedPeer)
    {
        std::cout << "LOBBY: Broadcasting data to all peers except " << excludedPeer->address.host << ":" << excludedPeer->address.port << "\n";

        for (size_t i = 0; i < m_server->peerCount; ++i)
        {
            ENetPeer* peer = &m_server->peers[i];
            if (peer != excludedPeer && peer->state == ENET_PEER_STATE_CONNECTED)
            {
                sendData(data, peer);
            }
        }
    }

    void broadcastDataExcept(ENetPacket* packet, ENetPeer* excludedPeer)
    {
        std::cout << "LOBBY: Broadcasting data to all peers except " << excludedPeer->address.host << ":" << excludedPeer->address.port << "\n";

        for (size_t i = 0; i < m_server->peerCount; ++i)
        {
            ENetPeer* peer = &m_server->peers[i];
            if (peer != excludedPeer && peer->state == ENET_PEER_STATE_CONNECTED)
            {
                sendData(packet, peer);
            }
        }
    }

    void sendData(const NetworkDatum& data, ENetPeer* clientPeer)
    {
        std::cout << "LOBBY: Sending data to client " << clientPeer->address.host << ":" << clientPeer->address.port << ": " << data << "\n";

        ENetPacket* packet = enet_packet_create(
            &data,
            sizeof(NetworkDatum),
            ENET_PACKET_FLAG_RELIABLE
        );

        if (!packet)
        {
            std::cerr << "LOBBY: Packet creation failed\n";
            return;
        }

        enet_peer_send(clientPeer, 0, packet);
        enet_host_flush(m_server);
    }

    void sendData(ENetPacket* packet, ENetPeer* clientPeer)
    {
        std::cout << "LOBBY: Sending data to client " << clientPeer->address.host << ":" << clientPeer->address.port << ": " << packet->data << "\n";

        enet_peer_send(clientPeer, 0, packet);
        enet_host_flush(m_server);
    }

    unsigned long concatenate(unsigned int x, unsigned int y)
    {
        x *= 10u;
        unsigned int temp = y;
        while (temp >= 10u)
        {
            temp /= 10u;
            x *= 10u;
        }

        return x + y;
    }
};
