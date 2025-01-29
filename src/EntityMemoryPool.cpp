#include "Entity.hpp"
#include "EntityMemoryPool.hpp"
#include "Components.hpp"
#include "Globals.hpp"

#include <vector>
#include <string>

#include <iostream>

/// @brief construct the entity memory pools (under one EntityMemoryPool object) and associated member variables
EntityMemoryPool::EntityMemoryPool(EntityID maxTiles, EntityID maxOtherEntities)
    : m_maxTiles(maxTiles),
    m_maxOtherEntities(maxOtherEntities),
    m_active(std::vector<bool>(maxTiles + maxOtherEntities))
{
    for (EntityID i = 0; i < maxTiles; ++i)
    {
        m_tileFreeList.push(i);
    }

    for (EntityID i = 0; i < maxOtherEntities; ++i)
    {
        m_otherEntityFreeList.push(i);
    }

    m_tilePool = std::make_tuple(
        std::vector<CColor>(maxTiles),
        // std::vector<CPosition>(maxTiles),
        // std::vector<CBoundingBox>(maxTiles),
        std::vector<CHealth>(maxTiles)
    );

    m_otherEntityPool = std::make_tuple(
        std::vector<CAnimation>(maxOtherEntities),
        std::vector<CTransform>(maxOtherEntities),
        std::vector<CBoundingBox>(maxOtherEntities),
        std::vector<CHealth>(maxOtherEntities),
        std::vector<CLifespan>(maxOtherEntities),
        std::vector<CDamage>(maxOtherEntities),
        std::vector<CInvincibility>(maxOtherEntities),
        std::vector<CInput>(maxOtherEntities),
        std::vector<CGravity>(maxOtherEntities),
        std::vector<CState>(maxOtherEntities),
        std::vector<CFireRate>(maxOtherEntities),
        std::vector<CColor>(maxOtherEntities)
        // std::vector<CPosition>(maxOtherEntities)
        // std::vector<CFollowPlayer>(maxOtherEntities),
        // std::vector<CPatrol>(maxOtherEntities)
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

    // m_pool = std::make_tuple(
    //     std::vector<CTransform>(maxEntities),
    //     std::vector<CAnimation>(maxEntities),
    //     std::vector<CBoundingBox>(maxEntities),
    //     std::vector<CHealth>(maxEntities),
    //     std::vector<CLifespan>(maxEntities),
    //     std::vector<CDamage>(maxEntities),
    //     std::vector<CInvincibility>(maxEntities),
    //     std::vector<CInput>(maxEntities),
    //     std::vector<CGravity>(maxEntities),
    //     std::vector<CState>(maxEntities),
    //     std::vector<CFireRate>(maxEntities),
    //     std::vector<CFollowPlayer>(maxEntities),
    //     std::vector<CPatrol>(maxEntities)
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
    static EntityMemoryPool pool(GlobalSettings::worldMaxCells.x * GlobalSettings::worldMaxCells.y, GlobalSettings::worldMaxEntities);
    return pool;
} // a singleton, globally acces in safe way, only one instance ever

/// TODO: could consider just returning unsigned long IDs everywhere instead of entity objects

/// @brief add an entity to the corresponding pool based on tag
Entity EntityMemoryPool::addEntity(const std::string& tag)
{
    std::cout << "adding entity" << std::endl;
    EntityID index;

    if (tag == "tile")
    {
        // find index
        if (!m_tileFreeList.empty())
        {
            index = m_tileFreeList.front();
            m_tileFreeList.pop();
        }
        else
        {
            std::cerr << "Tile memory pool full" << std::endl;
            exit(-1);
        }

        // reset tile at index
        std::apply([index](auto &...args)
            {
                ((args[index] = typename std::decay_t<decltype(args)>::value_type{}), ...);
            }, m_tilePool);

        // set active status
        m_active[index] = true;

        return Entity(index);
    }
    else
    {
        // find index
        if (!m_otherEntityFreeList.empty())
        {
            index = m_otherEntityFreeList.front();
            m_otherEntityFreeList.pop();
        }
        else
        {
            std::cerr << "Other entities memory pool full" << std::endl;
            exit(-1);
        }

        // reset entity at index
        std::apply([index](auto &...args)
            {
                ((args[index] = typename std::decay_t<decltype(args)>::value_type{}), ...);
            }, m_otherEntityPool);

        // set active status
        m_active[index + m_maxTiles] = true; // must add offset equal to max possible entity id for tiles

        return Entity(index + m_maxTiles); // must add offset equal to max possible entity id for tiles
    }
}

/// @brief remove an entity from its pool
void EntityMemoryPool::removeEntity(EntityID entityID)
{
    m_active[entityID] = false;

    if (entityID >= m_maxTiles)
    {
        m_otherEntityFreeList.push(entityID - m_maxTiles);
    }
    else
    {
        m_tileFreeList.push(entityID);
    }
}

/// @brief check to see if an entity is active
bool EntityMemoryPool::isActive(EntityID entityID)
{
    return m_active[entityID];
}
