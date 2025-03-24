#pragma once

#include "Globals.hpp"

#include <vector>
#include <cassert>

class NetEntityManager {
    std::vector<EntityID> m_freeList; // dynamic list tracking free IDs

public:

    /// @brief construct a NetEntityManager with a list of free IDs
    NetEntityManager() {
        for (int i = 0; i < GlobalSettings::worldMaxEntities; ++i) {
            m_freeList.push_back(i);
        }
    }

    /// @brief find and remove a free ID from the list of free IDs, return the ID
    EntityID addNetEntity() {
        assert(!m_freeList.empty());

        EntityID id{ m_freeList.back() };
        m_freeList.pop_back();

        return id;
    }

    /// @brief add id to the list of free IDs
    void makeFree(const EntityID id) {
        m_freeList.push_back(id);
    }
};
