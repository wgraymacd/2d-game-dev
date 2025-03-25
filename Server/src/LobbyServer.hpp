#pragma once

#include "Globals.hpp"
#include "LobbyEntityManager.hpp"

#include <enet/enet.h>

#include <vector>
#include <iostream>

class LobbyServer {
    ENetHost* m_server;
    int m_port;

    LobbyEntityManager m_lobbyEntityMan; // all entity IDs managed by this
    std::vector<EntityID> m_clientIDs; // list of players

    EntityID createNetEntity() {
        return m_lobbyEntityMan.addNetEntity();
    }

    void broadcastData(const NetworkData& data) {
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

    void sendData(ENetPeer* clientPeer, const NetworkData& data) {
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

public:

    LobbyServer(int port) : m_port(port) {
        m_clientIDs.reserve(Settings::maxLobbyPlayers);
    }

    void update() {
        ENetEvent event;
        enet_uint32 waitTime = 0; // ms, time to pause program until event received or time runs out (-1 for indefinite pause), will skip over events if they occur between checks, lower timout = more frequent calls to this function
        // enet_host_service checks for pending network events in a short time window and returns 0 if no events found during that window
        // polls events from an internal event queue maintained by ENet, network events are added to this queue
        // if queue empty and 0 timeout, immediately returns
        // if one event missed, then upon receiving next, both are processed
        while (enet_host_service(m_server, &event, waitTime) > 0) // enet_host_service polls for network events (&event: stores the next network event if one exists; 0: doesn't wait for new events)
        {
            NetworkData* received;

            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: /// @todo this event is not processed until the first message is sent from the client, could be a localhost thing, idk, but look into it later
                std::cout << "Client connected (host = " << event.peer->address.host << ", port = " << event.peer->address.port << ").\n";
                // store client info here if needed with event.peer->data

                break;

            case ENET_EVENT_TYPE_RECEIVE:
                received = (NetworkData*)event.packet->data;

                switch (received->dataType) {
                case POSITION:
                    std::cout << "Received POSITION: " << received->netID << ", " << received->first << ", " << received->second << "\n";
                    printf("Broadcasting position packet.\n");
                    enet_host_broadcast(m_server, 0, event.packet);
                    enet_host_flush(m_server); // automatically handles memory cleanup, cannot use with enet_packet_destroy
                    break;

                case VELOCITY:
                    std::cout << "Received VELOCITY: " << received->netID << ", " << received->first << ", " << received->second << "\n";
                    printf("Broadcasting velocity packet.\n");
                    enet_host_broadcast(m_server, 0, event.packet);
                    enet_host_flush(m_server);
                    break;

                case SPAWN:
                    std::cout << "Received SPAWN: " << received->localID << ", " << received->first << ", " << received->second << "\n";

                    std::cout << "Creating new net entity.\n";
                    received->netID = createNetEntity(); // network ID
                    received->dataType = LOCAL_SPAWN;
                    sendData(event.peer, *received);
                    received->dataType = SPAWN;
                    broadcastData(*received);
                    enet_packet_destroy(event.packet); // clean up memory after processing message, careful not to clean memory in use or already cleaned memory cuz will seg fault
                    break;

                case LOBBY_CONNECT:
                case LOCAL_SPAWN:
                case WORLD_SEED:
                    break;
                }

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

    bool isFull() const {
        return m_clientIDs.size() >= Settings::maxLobbyPlayers;
    }

    int getPort() const {
        return m_port;
    }
};
