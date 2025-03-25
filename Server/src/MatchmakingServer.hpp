#pragma once

#include "NetworkData.hpp"
#include "LobbyServer.hpp"

#include <enet/enet.h> 

#include <vector>
#include <iostream>

class MatchmakingServer {
private:
    ENetHost* m_server;

    std::vector<LobbyServer> m_activeLobbies;

    int m_nextLobbyPort = 5001; // start adding lobbies from port 5001

    void sendData(ENetPeer* clientPeer, const NetworkData& data) {
        std::cout << "Sending data to client: " << std::to_string(data.dataType) << ", " << data.localID << ", " << data.netID << ", " << data.first << ", " << data.second << "\n";

        ENetPacket* packet = enet_packet_create(
            &data,
            sizeof(NetworkData),
            ENET_PACKET_FLAG_RELIABLE
        );

        if (!packet) {
            std::cerr << "Packet creation failed\n";
            return;
        }

        enet_peer_send(clientPeer, 0, packet); // send to a single client
        enet_host_flush(m_server);
    }

    const LobbyServer& getFreeLoby() {
        // attempt to find existing lobby
        int i = 0;
        for (; i < m_activeLobbies.size(); ++i) {
            if (!m_activeLobbies[i].isFull()) {
                break;
            }
        }

        // otherwise, create new lobby
        if (i == m_activeLobbies.size()) {
            m_activeLobbies.emplace_back(m_nextLobbyPort++);
        }

        return m_activeLobbies[i];
    }

public:
    MatchmakingServer() {
        ENetAddress address;
        address.host = ENET_HOST_ANY; // accept connections from any IP address (host: device connected to a network, both server and client are hosts)
        address.port = 5000; // matchmaking port, listen for incoming connections on this port (port: number that IDs a specific process or service running on a host, clients must connects to this port)
        m_server = enet_host_create(&address, 32, 2, 0, 0); // 32 clients and/or outgoing connections, 2 channels used (0 and 1, e.g., one for game state and one for chat), any amount of incoming bandwidth, any amount of outgoing bandwidth
        if (m_server == nullptr) {
            std::cerr << "Failed to create matchmaking server" << std::endl;
            exit(1);
        }

        std::cout << "Server started: address.host = " << address.host << " and address.port = " << address.port << "\n";
        /// TODO: put this output stuff into a log file instead of console, then can have simple message like "Stop server? (Y/n)" 
    }

    ~MatchmakingServer() {
        if (m_server) {
            enet_host_destroy(m_server);
        }
    }

    void update() {
        ENetEvent event;
        enet_uint32 waitTime = 1000; // ms, time to pause program until event received or time runs out (-1 for indefinite pause), will skip over events if they occur between checks, lower timout = more frequent calls to this function
        // enet_host_service checks for pending network events in a short time window and returns 0 if no events found during that window
        // polls events from an internal event queue maintained by ENet, network events are added to this queue
        // if queue empty and 0 timeout, immediately returns
        // if one event missed, then upon receiving next, both are processed
        while (enet_host_service(m_server, &event, waitTime) > 0) { // enet_host_service polls for network events (&event: stores the next network event if one exists; 0: doesn't wait for new events)
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "Matchmaking: client connected (host = " << event.peer->address.host << ", port = " << event.peer->address.port << ")\n";
                break;

            case ENET_EVENT_TYPE_RECEIVE: {
                NetworkData* received = (NetworkData*)event.packet->data;

                if (received->dataType == LOBBY_CONNECT) {
                    std::cout << "Received LOBBY_CONNECT\n";

                    // send client the lobby data so client can connect
                    const LobbyServer& lobby = getFreeLoby();
                    NetworkData data{ .dataType = LOBBY_CONNECT, .netID = lobby.getPort() };
                    sendData(event.peer, data);

                    /// @todo
                    // client attempts to connect to lobby
                    // lobby creates unique client ID for lobby and sends to client
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
    std::vector<LobbyServer>& getActiveLobbies() {
        return m_activeLobbies;
    }
};
