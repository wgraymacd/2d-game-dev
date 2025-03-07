#pragma once

#include "Globals.hpp"

#include <vector>
#include <stack>

class NetEntityManager
{
    // std::vector<bool> m_netEntities = std::vector<bool>(GlobalSettings::worldMaxEntities); // true if active, false if not, where ID = index
    std::vector<NetEntityID> m_freeList; // dynamic list tracking free spaces in m_netEntities

public:

    NetEntityManager()
    {
        for (int i = 0; i < GlobalSettings::worldMaxEntities; ++i)
        {
            m_freeList.push_back(i);
        }
    }

    NetEntityID addNetEntity()
    {
        NetEntityID id;

        // find index
        if (!m_freeList.empty())
        {
            id = m_freeList.back();
            m_freeList.pop_back();
        }
        else
        {
            printf("Free list empty.");
            exit(EXIT_FAILURE);
        }

        // m_netEntities[id] = true;
        return id;
    }

    void removeNetEntity(const NetEntityID id)
    {
        // m_netEntities[id] = false;
        m_freeList.push_back(id);
    }
};