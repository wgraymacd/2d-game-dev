// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Global
#include "EntityBase.hpp"

// C++ Standard Libraries
#include <cstdint>
#include <ostream>
#include <array>
#include <string_view>

struct NetworkDatum
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

        // server-to-client: first.id = network entity ID, second.type = entity type, third.f = x pos, fourth.f = y pos
        // client-to-server: first.id = local entity ID, second.type = entity type, third.f = x pos, fourth.f = y pos
        SPAWN,

        // server-to-client: first.id = local entity ID, second.id = network entity ID
        // client-to-server: -
        LOCAL_SPAWN,

        // server-to-client: first.id = network entity ID
        // client-to-server: first.id = network entity ID
        DESPAWN,

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
        EntityBase::Type type;
        int i;
        float f;
    }
    first { };

    union
    {
        EntityID id;
        EntityBase::Type type;
        int i;
        float f;
    }
    second { };

    union
    {
        EntityID id;
        EntityBase::Type type;
        int i;
        float f;
    }
    third { };

    union
    {
        EntityID id;
        EntityBase::Type type;
        int i;
        float f;
    }
    fourth { };

    union
    {
        EntityID id;
        EntityBase::Type type;
        int i;
        float f;
    }
    fifth { };
};

/// @todo change this to a .cpp file maybe, inline is not great since this is a semi-complex function, making this inline will allow multiple definitions of it, remove for release builds
/// @todo remove this for release builds, this is just for debugging
inline std::ostream& operator<<(std::ostream& out, const NetworkDatum& netDatum)
{
    out << "Data Type: " << static_cast<int>(netDatum.dataType);

    if (netDatum.dataType == NetworkDatum::DataType::POSITION ||
        netDatum.dataType == NetworkDatum::DataType::VELOCITY)
        out << ", Net ID: " << netDatum.first.id << ", x: " << netDatum.second.f << ", y: " << netDatum.third.f;
    else if (netDatum.dataType == NetworkDatum::DataType::SPAWN)
        out << ", ID: " << netDatum.first.id << ", type: " << static_cast<EntityBase::Type>(netDatum.second.type) << ", x: " << netDatum.third.f << ", y: " << netDatum.fourth.f;
    else if (netDatum.dataType == NetworkDatum::DataType::LOCAL_SPAWN)
        out << ", Local ID: " << netDatum.first.id << ", Net ID: " << netDatum.second.id;
    else if (netDatum.dataType == NetworkDatum::DataType::DESPAWN)
        out << ", Net ID: " << netDatum.first.id;
    else if (netDatum.dataType == NetworkDatum::DataType::WORLD_SEED)
        out << ", Seed: " << netDatum.first.i;
    else if (netDatum.dataType == NetworkDatum::DataType::LOBBY_CONNECT)
        out << ", Address:Port: " << netDatum.first.i << "." << netDatum.second.i << "." << netDatum.third.i << "." << netDatum.fourth.i << ":" << netDatum.fifth.i;

    return out;
}
