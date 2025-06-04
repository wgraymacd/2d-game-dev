// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

// Core
#include "Entity.hpp"
#include "EntityMemoryPool.hpp"

// Global
#include "Timer.hpp"

// C++ standard libraries
#include <string>
#include <vector>

class EntityManager
{
    std::vector<std::pair<Entity::Type, Entity>> m_entitiesToAdd;
    std::array<std::vector<Entity>, Entity::Type::NUM_TYPES> m_entityVecArr; // collidable layer entities without a dedicated layer matrix (player, bullet, weapon, npc, etc.), NO TILES

    /// @brief remove dead entities from param entities
    void removeDeadEntities()
    {
        PROFILE_FUNCTION();

        // remove from m_liveEntities
        // auto it = std::remove_if(m_liveEntities.begin(), m_liveEntities.end(),
        //     [](const Entity& entity) { return !entity.isActive(); });
        // m_liveEntities.erase(it, m_liveEntities.end());

        // remove from m_entityVecArr
        /// TODO: super slow if m_entityVecArr becomes large
        /// TODO: try the locational thing to the player like the tileMatrix[x][y]? try pre initialized vectors and just use active vs inactive? remove map entirely in favor of different memory pools or something for each type of entity?
        for (std::vector<Entity>& entityVec : m_entityVecArr)
        {
            auto itMap = std::remove_if(entityVec.begin(), entityVec.end(), [](const Entity& entity) { return !entity.isActive(); });
            entityVec.erase(itMap, entityVec.end());
        }
    }

public:
    EntityManager() = default;

    /// TODO: implement new version of this if needed
    /// @brief adds entities to be added and removes entities to be destroyed
    void update()
    {
        PROFILE_FUNCTION();

        for (std::pair<Entity::Type, Entity>& pair : m_entitiesToAdd)
        {
            m_entityVecArr[pair.first].push_back(pair.second);
        }
        m_entitiesToAdd.clear();

        removeDeadEntities();
    }

    /// @brief marks new Entity to be added on next call to EntityManager::update, returns new Entity
    Entity addEntity(Entity::Type type)
    {
        // PROFILE_FUNCTION();

        Entity e = EntityMemoryPool::Instance().addEntity();
        m_entitiesToAdd.emplace_back(type, e);
        return e;
    }

    /// TODO: implement the new version of this if needed, also could change this to return only entity IDs to not have to create a bunch of new entities (but consider how that affects usage in ScenePlay and so on), make this return a reference with data stored in memory if needed
    /// @brief gets all entities with a specified tag
    /// @return an std::vector<Entity> of entities with the specified tag
    std::vector<Entity>& getEntities(Entity::Type type)
    {
        assert(type < Entity::Type::NUM_TYPES);

        return m_entityVecArr[type];
    }

    /// @brief get a single entity from its ID, for use only when entity ID must be used over entity, only use when attempting to access components of a known, existing entity with entityID id
    Entity getEntity(EntityID id) const
    {
        return Entity(id);
    }

    /// TODO: implement the new version of this if needed
    /// @brief gets the map of entity tags to entities
    /// @return m_entityVecArr, the map of Entity::Type tags to std::vector<Entity> entity vectors
    const std::array<std::vector<Entity>, Entity::Type::NUM_TYPES>& getEntityVecArr()
    {
        return m_entityVecArr;
    }
};
