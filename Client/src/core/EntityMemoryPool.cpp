// Copyright 2025, William MacDonald, All Rights Reserved.

// Core
#include "Entity.hpp"
#include "EntityMemoryPool.hpp"
#include "Components.hpp"

// Utility
#include "utility/ClientGlobals.hpp"

// C++ standard libraries
#include <vector>
#include <string>
#include <iostream>

/// @brief construct the entity memory pools (under one EntityMemoryPool object) and associated member variables
EntityMemoryPool::EntityMemoryPool(EntityID maxEntities)
    : m_maxEntities(maxEntities),
    m_active(std::vector<bool>(maxEntities))
{

    for (EntityID i = 0; i < maxEntities; ++i)
    {
        m_entityFreeList.push(i);
    }

    m_pool = std::make_tuple(
        std::vector<CAnimation>(maxEntities),
        std::vector<CTransform>(maxEntities),
        std::vector<CBoundingBox>(maxEntities),
        std::vector<CHealth>(maxEntities),
        std::vector<CLifespan>(maxEntities),
        std::vector<CDamage>(maxEntities),
        std::vector<CInvincibility>(maxEntities),
        std::vector<CInput>(maxEntities),
        std::vector<CGravity>(maxEntities),
        std::vector<CState>(maxEntities),
        std::vector<CFire>(maxEntities),
        std::vector<CJointRelation>(maxEntities),
        std::vector<CJointInfo>(maxEntities),
        std::vector<CSkelAnim>(maxEntities)
    );

    // m_pool = std::make_tuple(
    //     std::vector<CTransform>(maxEntities),
    //     std::vector<CAnimation>(maxEntities),
    //     std::vector<CBoundingBox>(maxEntities),
    //     std::vector<CHealth>(maxEntities),
    //     std::unordered_map<EntityID, CLifespan>(),
    //     std::unordered_map<EntityID, CDamage>(),
    //     std::unordered_map<EntityID, CInvincibility>(),
    //     std::unordered_map<EntityID, CInput>(),
    //     std::unordered_map<EntityID, CGravity>(),
    //     std::unordered_map<EntityID, CState>(),
    //     std::unordered_map<EntityID, CFireRate>(),
    //     std::unordered_map<EntityID, CFollowPlayer>(),
    //     std::unordered_map<EntityID, CPatrol>()
    // );
}

/// vector and unordered map in same pool method
// void EntityMemoryPool::resetEntityAtIndex(EntityID index)
// {
    // std::apply([this, index](auto &...args)
    //     {
    //         // Iterate through each container in the tuple
    //         ((resetContainer(index, args)), ...); // Using fold expression to apply resetContainer to each container
    //     }, m_pool);
// }

/// @brief singleton class for entity memory pool
/// @return same, persistent instance of the class
EntityMemoryPool& EntityMemoryPool::Instance()
{
    static EntityMemoryPool pool(Settings::worldMaxEntities);
    // static EntityMemoryPool pool(Settings::worldMaxCells.x * Settings::worldMaxCells.y, Settings::worldMaxEntities);
    return pool;
} // a singleton, globally acces in safe way, only one instance ever

/// TODO: could consider just returning IDs everywhere instead of entity objects

Entity EntityMemoryPool::addEntity()
{
    EntityID index;

    // find index
    if (!m_entityFreeList.empty())
    {
        index = m_entityFreeList.front();
        m_entityFreeList.pop();
    }
    else
    {
        std::cerr << "Other entities memory pool full" << std::endl;
        exit(-1);
    }

    // just set exists flag to false at index, no need to create default constructed values
    std::apply([index](auto &...args)
        {
            ((args[index].exists = false), ...);  // reset "exists" flag for each component
        }, m_pool);

    // set active status
    m_active[index] = true;

    return Entity(index);
}

/// @brief remove an entity from its pool
void EntityMemoryPool::removeEntity(EntityID entityID)
{
    m_active[entityID] = false;

    m_entityFreeList.push(entityID);
}

/// @brief check to see if an entity is active
bool EntityMemoryPool::isActive(EntityID entityID)
{
    return m_active[entityID];
}
