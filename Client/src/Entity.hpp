// Copyright 2025, William MacDonald, All Rights Reserved.

#pragma once

#include "EntityMemoryPool.hpp"
#include "Globals.hpp"
#include "Timer.hpp"

/// TODO: consider adding subtypes of entities like Tiles so that entity ids (check EntityMemoryPool) don't have to be offset for different memory pools (requires taking entityID - maxTiles as index to memory pool after tile memory pool)

class Entity
{
    EntityID m_id = -1;
    Entity(EntityID id);
    friend class EntityMemoryPool;
    friend class EntityManager; // so let entity manager create entities from entity IDs

public:
    /// TODO: is this bad? need a new way of doing this so that Entity constructor stays private, maybe create new files for tile management and new classes inheriting from entity like Tile with a private constructor in the same file as the tile matrix functions
    Entity() = default; // for initialization of tile matrix in entity manager, will create an m_id = -1

    /// @brief get a component of type T from this entity
    template <typename T>
    T& getComponent() const
    {
        // PROFILE_FUNCTION();

        return EntityMemoryPool::Instance().getComponent<T>(m_id);
    }

    /// @brief check to see if this entity has a component of type T
    template <typename T>
    const bool hasComponent() const
    {
        // PROFILE_FUNCTION();

        return EntityMemoryPool::Instance().hasComponent<T>(m_id);
    }

    /// @brief add a component of type T with argument mArgs of types TArgs to this entity
    template <typename T, typename... TArgs>
    T& addComponent(TArgs &&...mArgs)
    {
        // PROFILE_FUNCTION();

        return EntityMemoryPool::Instance().addComponent<T>(m_id, std::forward<TArgs>(mArgs)...);
    }

    void destroy() const;
    const bool isActive() const;
    const EntityID getID() const;
};