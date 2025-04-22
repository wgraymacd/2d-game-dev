// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Server
#include "ServerGlobals.hpp"

// C++ standard library
#include <vector>
#include <cassert>

class LobbyEntityManager
{
public:

    LobbyEntityManager()
    {
        for (EntityID id = 0; id < m_maxPlayers; ++id)
        {
            m_freeList.push_back(id);
        }
    }

    /// @brief find and remove a free ID from the list of free IDs, return the ID
    EntityID addNetEntity()
    {
        assert(!m_freeList.empty());

        EntityID id { m_freeList.back() };
        m_freeList.pop_back();

        return id;
    }

    /// @brief add id to the list of free IDs
    void makeFree(EntityID id)
    {
        m_freeList.push_back(id);
    }

private:

    std::vector<EntityID> m_freeList; // dynamic list tracking free IDs
    const EntityID m_maxPlayers { Settings::worldMaxEntities };
};
