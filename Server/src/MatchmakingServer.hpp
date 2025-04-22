// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Server
#include "LobbyServer.hpp"

// Global
#include "NetworkData.hpp"

// External libraries
#include <enet/enet.h>

// C++ standard library
#include <vector>
#include <iostream>

class MatchmakingServer
{
public:
    MatchmakingServer()
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY; // Accept connections from any IP address (host: device connected to a network, both server and client are hosts)
        address.port = 5000; // Matchmaking port, listen for incoming connections on this port (port: number that IDs a specific process or service running on a host, clients must connects to this port)
        m_server = enet_host_create(&address, 32, 2, 0, 0); // 32 clients and/or outgoing connections, 2 channels used (0 and 1, e.g., one for game state and one for chat), any amount of incoming bandwidth, any amount of outgoing bandwidth
        if (m_server == nullptr)
        {
            std::cerr << "Failed to create matchmaking server" << std::endl;
            exit(1);
        }

        std::cout << "Server started: address.host = " << address.host << " and address.port = " << address.port << "\n";
        /// TODO: put this output stuff into a log file instead of console, then can have simple message like "Stop server? (Y/n)"
    }

    ~MatchmakingServer()
    {
        if (m_server)
        {
            enet_host_destroy(m_server);
        }
    }

    void update()
    {
        ENetEvent event;

        // enet_host_service checks for pending network events in a short time window and returns 0 if no events found during that window
        // polls events from an internal event queue maintained by ENet, network events are added to this queue
        // if queue empty and 0 timeout, immediately returns
        // if one event missed, then upon receiving next, both are processed
        // 1000 ms to pause program until event received or time runs out (-1 for indefinite pause), will skip over events if they occur between checks, lower timout = more frequent calls to this function)
        while (enet_host_service(m_server, &event, static_cast<enet_uint32>(1000)) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Matchmaking: client connected to " << event.peer->address.host << ":" << event.peer->address.port << "\n";
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                {
                    const NetworkData* const received = (NetworkData*)event.packet->data;

                    if (received->dataType == NetworkData::DataType::LOBBY_CONNECT)
                    {
                        std::cout << "Received LOBBY_CONNECT\n";

                        /// @todo change this from being hardcoded to 127.0.0.1
                        const LobbyServer& lobby = getFreeLoby();
                        NetworkData data {
                            NetworkData::DataType::LOBBY_CONNECT,
                            {.i = 127 },
                            {.i = 0 },
                            {.i = 0 },
                            {.i = 1 },
                            {.i = lobby.getPort() }
                        };
                        sendData(event.peer, data);
                    }

                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Matchmaking: client disconnected\n";
                    break;

                case ENET_EVENT_TYPE_NONE:
                    break;

                default:
                    break;
            }
        }
    }

    // added by chat
    std::vector<LobbyServer>& getActiveLobbies()
    {
        return m_activeLobbies;
    }

private:

    ENetHost* m_server;

    std::vector<LobbyServer> m_activeLobbies;

    int m_nextLobbyPort = 5001; // Start adding lobbies from port 5001

    void sendData(ENetPeer* clientPeer, const NetworkData& data)
    {
        std::cout << "Sending data to client: " << std::to_string(data.dataType) << "\n";

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

    const LobbyServer& getFreeLoby()
    {
        // Attempt to find existing lobby
        for (const LobbyServer& lobby : m_activeLobbies)
        {
            if (!lobby.isFull())
            {
                return lobby;
            }
        }

        // Otherwise, create new lobby
        m_activeLobbies.emplace_back(m_nextLobbyPort++);
        return m_activeLobbies.back();
    }
};
