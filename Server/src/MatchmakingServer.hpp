// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Server
#include "LobbyServer.hpp"

// Global
#include "NetworkDatum.hpp"

// External libraries
#include <enet/enet.h>

// C++ standard library
#include <vector>
#include <iostream>
#include <thread>
// #include <mutex> // Add this for thread safety

class MatchmakingServer
{
public:

    // std::mutex lobbiesMutex; // Mutex for thread safety

    MatchmakingServer()
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY; // Accept connections from any IP address (host: device connected to a network, both server and client are hosts)
        address.port = 5000; // Matchmaking port, listen for incoming connections on this port (port: number that IDs a specific process or service running on a host, clients must connects to this port)
        m_server = enet_host_create(&address, 32, 2, 0, 0); // 32 clients and/or outgoing connections, 2 channels used (0 and 1, e.g., one for game state and one for chat), any amount of incoming bandwidth, any amount of outgoing bandwidth
        if (m_server == nullptr)
        {
            std::cerr << "MATCHMAKING: Failed to create matchmaking server" << std::endl;
            exit(1);
        }

        std::cout << "MATCHMAKING: Server started at " << address.host << ":" << address.port << "\n";
        /// TODO: put this output stuff into a log file instead of console, then can have simple message like "Stop server? (Y/n)"
    }

    ~MatchmakingServer()
    {
        // Signal the lobby threads to stop
        m_isRunning = false;

        // Join the lobby threads
        for (std::thread& thread : m_lobbyThreads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        // Clean up the ENet resources
        if (m_server)
        {
            enet_host_destroy(m_server);
        }

        std::cout << "MATCHMAKING: Server stopped\n";
    }

    void update()
    {
        ENetEvent event;

        // enet_host_service checks for pending network events in a short time window and returns 0 if no events found during that window
        // polls events from an internal event queue maintained by ENet, network events are added to this queue
        // if queue empty and 0 timeout, immediately returns
        // if one event missed, then upon receiving next, both are processed
        // 1000 ms to pause program until event received or time runs out (-1 for indefinite pause), will skip over events if they occur between checks, lower timout = more frequent calls to this function)
        while (enet_host_service(m_server, &event, 1000) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "MATCHMAKING: Client connected from " << event.peer->address.host << ":" << event.peer->address.port << "\n";
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                {
                    NetworkDatum received;
                    std::memcpy(&received, event.packet->data, sizeof(NetworkDatum));
                    std::cout << "MATCHMAKING: Received data: " << received << " from " << event.peer->address.host << ":" << event.peer->address.port << "\n";

                    if (received.dataType == NetworkDatum::DataType::LOBBY_CONNECT)
                    {
                        /// @todo change this from being hardcoded to 127.0.0.1
                        const LobbyServer* lobby = getFreeLoby();
                        NetworkDatum data {
                            NetworkDatum::DataType::LOBBY_CONNECT,
                            .first.i = 127,
                            .second.i = 0,
                            .third.i = 0,
                            .fourth.i = 1,
                            .fifth.i = lobby->getPort()
                        };
                        sendData(event.peer, data);
                    }

                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "MATCHMAKING: Client at " << event.peer->address.host << ":" << event.peer->address.port << " disconnected\n";
                    break;

                case ENET_EVENT_TYPE_NONE:
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
    std::vector<std::thread> m_lobbyThreads;

    int m_nextLobbyPort = 5001; // Start adding lobbies from port 5001

    bool m_isRunning = true;

    void sendData(ENetPeer* clientPeer, const NetworkDatum& data)
    {
        std::cout << "MATCHMAKING: Sending data to client at " << clientPeer->address.host << ":" << clientPeer->address.port << ": " << data << "\n";

        ENetPacket* packet = enet_packet_create(
            &data,
            sizeof(NetworkDatum),
            ENET_PACKET_FLAG_RELIABLE
        );

        if (!packet)
        {
            std::cerr << "MATCHMAKING: Packet creation failed\n";
            return;
        }

        enet_peer_send(clientPeer, 0, packet); // Send to a single client
        enet_host_flush(m_server);
    }

    LobbyServer* getFreeLoby()
    {
        // std::lock_guard<std::mutex> lock(lobbiesMutex); // don't think I need this here, as this function is only called from the main thread

        // Attempt to find an existing lobby
        for (LobbyServer& lobby : m_activeLobbies)
        {
            if (!lobby.isFull())
            {
                return &lobby;
            }
        }

        // Otherwise, create a new lobby
        m_activeLobbies.emplace_back(m_nextLobbyPort++);
        m_lobbyThreads.emplace_back(&MatchmakingServer::runLobby, this, std::ref(m_activeLobbies.back()));
        return &m_activeLobbies.back();
    }

    void runLobby(LobbyServer& lobby)
    {
        while (m_isRunning)
        {
            lobby.update();
        }
    }
};
