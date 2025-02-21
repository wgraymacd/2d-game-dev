#include "Entity.hpp"
#include "EntityMemoryPool.hpp"

#include <string>

/// TODO: consider adding subtypes of entities like Tiles so that entity ids (check EntityMemoryPool) don't have to be offset for different memory pools (requires taking entityID - maxTiles as index to memory pool after tile memory pool)

/// TODO: is this bad? need a new way of doing this so that Entity constructor stays private, maybe create new files for tile management and new classes inheriting from entity like Tile with a private constructor in the same file as the tile matrix functions
Entity::Entity(EntityID id) : m_id(id) {}

/// @brief destroy this entity
void Entity::destroy() const
{
    EntityMemoryPool::Instance().removeEntity(m_id);
}

/// @brief get a bool representing this entities living status
const bool Entity::isActive() const
{
    return EntityMemoryPool::Instance().isActive(m_id);
}

const EntityID Entity::getID() const
{
    return m_id;
}