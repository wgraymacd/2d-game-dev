#pragma once

#include "Components.hpp"
#include "EntityMemoryPool.hpp"
#include <string>
#include <tuple>

class Entity
{
    unsigned long m_id = -1;

    Entity(unsigned long id) : m_id(id) {}

    friend class EntityMemoryPool;

public:
    /// TODO: is this bad? need a new way of doing this so that Entity constructor stays private, maybe create new files for tile management and new classes inheriting from entity like Tile with a private constructor in the same file as the tile matrix functions
    Entity() = default; // for initialization of tile matrix in entity manager, will create an m_id = -1

    /// @brief get a component of type T from this entity
    template <typename T>
    T& getComponent() const
    {
        return EntityMemoryPool::Instance().getComponent<T>(m_id);
    }

    /// @brief check to see if this entity has a component of type T
    template <typename T>
    const bool hasComponent() const
    {
        return EntityMemoryPool::Instance().hasComponent<T>(m_id);
    }

    /// @brief add a component of type T with argument mArgs of types TArgs to this entity
    template <typename T, typename... TArgs>
    T& addComponent(TArgs &&...mArgs)
    {
        return EntityMemoryPool::Instance().addComponent<T>(m_id, std::forward<TArgs>(mArgs)...);
    }

    /// @brief get this entity's tag
    const std::string tag()
    {
        return EntityMemoryPool::Instance().getTag(m_id);
    }

    /// @brief destroy this entity
    void destroy() const
    {
        EntityMemoryPool::Instance().removeEntity(m_id);
    }

    /// @brief get a bool representing this entities living status
    const bool isActive() const
    {
        return EntityMemoryPool::Instance().isActive(m_id);
    }
};