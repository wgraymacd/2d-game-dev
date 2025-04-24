// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "Globals.hpp"

#include <cstdint>
#include <ostream>
#include <array>
#include <string_view>

struct NetworkData
{
    enum DataType : uint8_t
    {
        NONE,

        // server-to-client: first.id = network entity ID, second.f = x pos, third.f = y pos
        // client-to-server: first.id = network entity ID, second.f = x pos, third.f = y pos
        POSITION,

        // server-to-client: first.id = network entity ID, second.f = x vel, third.f = y vel
        // client-to-server: first.id = network entity ID, second.f = x vel, third.f = y vel
        VELOCITY,

        // server-to-client: first.id = network entity ID, second.f = x pos, third.f = y pos
        // client-to-server: first.id = local entity ID, second.f = x pos, third.f = y pos
        SPAWN,

        // server-to-client: first.id = local entity ID, second.id = network entity ID
        // client-to-server: -
        LOCAL_SPAWN,

        // server-to-client: first.i = world seed
        // client-to-server: -
        WORLD_SEED,

        // server-to-client: first.i = lobby IP part 1, second.i = lobby IP part 2, third.i = lobby IP part 3, forth.i = lobby IP part 4, fifth.i = lobby port
        // client-to-server: -
        LOBBY_CONNECT,

        // the number of data types (including NONE)
        NUM_TYPES
    }
    dataType { NONE };

    union
    {
        EntityID id;
        int i;
        float f;
    }
    first { };

    union
    {
        EntityID id;
        int i;
        float f;
    }
    second { };

    union
    {
        EntityID id;
        int i;
        float f;
    }
    third { };

    union
    {
        EntityID id;
        int i;
        float f;
    }
    fourth { };

    union
    {
        EntityID id;
        int i;
        float f;
    }
    fifth { };
};

/// @todo remove this for release builds, this is just for debugging
namespace Network
{
    constexpr std::array<std::string_view, NetworkData::DataType::NUM_TYPES> dataTypes = { "NONE", "POSITION", "VELOCITY", "SPAWN", "LOCAL_SPAWN", "WORLD_SEED", "LOBBY_CONNECT" };
}

/// @todo change this to a .cpp file maybe, inline is not great since this is a semi-complex function, making this inline will allow multiple definitions of it, remove for release builds
/// @todo remove this for release builds, this is just for debugging
inline std::ostream& operator<<(std::ostream& out, const NetworkData& netData)
{
    out << "Data Type: " << Network::dataTypes[netData.dataType];

    if (netData.dataType == NetworkData::DataType::POSITION ||
        netData.dataType == NetworkData::DataType::VELOCITY ||
        netData.dataType == NetworkData::DataType::SPAWN)
        out << ", ID: " << netData.first.id << ", x: " << netData.second.f << ", y: " << netData.third.f;
    else if (netData.dataType == NetworkData::DataType::LOCAL_SPAWN)
        out << ", Net ID: " << netData.first.id << ", Local ID: " << netData.second.id;
    else if (netData.dataType == NetworkData::DataType::WORLD_SEED)
        out << ", Seed: " << netData.first.i;
    else if (netData.dataType == NetworkData::DataType::LOBBY_CONNECT)
        out << ", Address:Port: " << netData.first.i << "." << netData.second.i << "." << netData.third.i << "." << netData.fourth.i << ":" << netData.fifth.i;

    return out;
}
