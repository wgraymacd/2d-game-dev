// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Server
#include "ServerGlobals.hpp"

// Global
#include "EntityBase.hpp"
#include "NetworkDatum.hpp"

// C++ standard library
#include <vector>
#include <cassert>

class LobbyEntityManager
{
public:

    LobbyEntityManager()
    {
        for (EntityID id = 0; id < m_maxEntities; ++id)
        {
            m_freeList.push_back(id);
        }
    }

    /// @brief find and remove a free ID from the list of free IDs, return the ID
    EntityID addNetEntity(NetworkDatum datum)
    {
        assert(!m_freeList.empty());

        EntityID id { m_freeList.back() };
        m_freeList.pop_back();

        // Modify datum and add to current state
        datum.first.id = id;
        if (datum.second.type == EntityBase::Type::PLAYER)
        {
            datum.second.type = EntityBase::Type::ENEMY; // to send to future joining clients
        }
        m_currentState[id] = datum;

        // Add id to m_active
        m_active.push_back(id);

        return id;
    }

    /// @brief add id to the list of free IDs
    void destroy(EntityID id)
    {
        // Add id to free list
        m_freeList.push_back(id);

        // Erase id from m_active
        auto it = std::find(m_active.begin(), m_active.end(), id);
        if (it != m_active.end())
        {
            m_active.erase(it);
        }
    }

    const std::array<NetworkDatum, Settings::worldMaxEntities>& getCurrentState() const
    {
        return m_currentState;
    }

    const std::vector<EntityID>& getActiveEntities() const
    {
        return m_active;
    }

private:

    const EntityID m_maxEntities { Settings::worldMaxEntities };
    std::vector<EntityID> m_freeList; // dynamic list tracking free IDs
    std::vector<EntityID> m_active; // dynamic list holding IDs of active entities for indexing current state
    /// @todo consider making this an unordered_map or smthn instead of an array for less memory usage
    std::array<NetworkDatum, Settings::worldMaxEntities> m_currentState; // list of entity data to send to joining clients (SPAWN type)

    /// TODO: consider making an entity memory pool type thing but with only the network data I need to send to the clients to keep track of lots of state data like all tiles and entities' status, otherwise keep this method of using an array with all of it and the m_active vector
};
