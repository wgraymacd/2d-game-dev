#include "Entity.hpp"
#include "EntityMemoryPool.hpp"
#include "Components.hpp"

#include <vector>
#include <string>

#include <iostream>

/// privates

EntityMemoryPool::EntityMemoryPool(unsigned long maxEntities)
{
    for (unsigned long i = 0; i < maxEntities; ++i)
    {
        m_freeList.push(i);
    }
}

unsigned long EntityMemoryPool::getInactiveEntityIndex()
{
    unsigned long index = m_freeList.front();
    m_freeList.pop();
    return index;
}

void EntityMemoryPool::resetEntityAtIndex(unsigned long index)
{
    std::apply([index](auto &...args)
        { ((args[index] = typename std::decay_t<decltype(args)>::value_type{}), ...); }, m_pool);
}

/// publics

EntityMemoryPool& EntityMemoryPool::Instance()
{
    static EntityMemoryPool pool(MAX_ENTITIES);
    return pool;
} // a singleton, globally acces in safe way, only one instance ever

// std::vector<Entity> EntityMemoryPool::getEntities()
// {

// }

// std::vector<Entity> EntityMemoryPool::getEntities(const std::string &tag)
// {

// }

const std::string& EntityMemoryPool::getTag(unsigned long entityID) const
{
    return m_tags[entityID];
}

/// TODO: When entities are created, you clear the components for that entity in the vectors. However, if certain components have non-trivial constructors, you might want to ensure that they are properly initialized when an entity is created
/// TODO: could consider just returning unsigned long IDs everywhere instead of entity objects
Entity EntityMemoryPool::addEntity(const std::string& tag)
{
    unsigned long index = getInactiveEntityIndex();

    resetEntityAtIndex(index);

    m_tags[index] = tag;
    m_active[index] = true;

    ++m_numEntities;

    return Entity(index);
}

void EntityMemoryPool::removeEntity(unsigned long entityID)
{
    m_active[entityID] = false;
    m_freeList.push(entityID);
}

/// TODO: is this the most efficient way? Or use m_entityMap in EntityManager
std::vector<Entity> EntityMemoryPool::getEntities(const std::string& tag)
{
    std::vector<Entity> vec;
    for (int i = 0; i < m_tags.size(); ++i)
    {
        if (m_tags[i] == tag)
        {
            vec.push_back(Entity(i));
        }
    }
    return vec;
}

bool EntityMemoryPool::isActive(unsigned long entityID)
{
    return m_active[entityID];
}
