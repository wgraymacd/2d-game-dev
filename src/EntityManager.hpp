#pragma once

#include "Entity.hpp"
#include "EntityMemoryPool.hpp"
#include <string>
#include <vector>

class EntityManager
{
    /// TODO: still needed? think about this in relation to the new memory pool
    std::vector<Entity> m_entities;
    std::vector<Entity> m_entitiesToAdd;
    // std::map<std::string, std::vector<Entity>> m_entityMap;
    unsigned long m_totalEntities = 0;

    /// TODO: implement
    void removeDeadEntities(std::vector<Entity> &vec)
    {
        
    }

public:
    EntityManager() = default;

    /// TODO: implement new version of this if needed
    /// @brief adds entities to be added and removed entities to be destroyed
    void update()
    {
        // bool print = false;
        // if (m_entitiesToAdd.size() > 0)
        // {
        //     print = true;
        // }

        for (auto& e : m_entitiesToAdd)
        {
            m_entities.push_back(e);
            // m_entityMap[e.tag()].push_back(e);
        }
        m_entitiesToAdd.clear();

        removeDeadEntities(m_entities);

        // for (auto &[tag, entityVec] : m_entityMap)
        // {
        //     removeDeadEntities(entityVec);
        // }

        // if (print)
        // {
        //     std::cout << "\nin update call, num of entites: " << m_entities.size() << "\n";
        //     for (int i = 0; i < m_entities.size(); i++)
        //     {
        //         std::cout << "entity manager updated: " << m_entities[i]->tag() << "\n";
        //     }
        //     std::cout << "\nentity map:\n";
        //     for (auto &[tag, entityVec] : m_entityMap)
        //     {
        //         std::cout << tag << "\n";
        //         for (auto &e : entityVec)
        //         {
        //             std::cout << "    " << e->id() << "\n";
        //         }
        //     }
        // }
    }

    /// @brief marks new entity to be added on next call to EntityManager::update
    /// @param tag the type of entity to add (e.g., "player")
    /// @return the entity to be added
    Entity addEntity(const std::string &tag)
    {
        Entity e = EntityMemoryPool::Instance().addEntity(tag);
        m_entitiesToAdd.push_back(e);
        return e;
    }

    /// TODO: implement the new version of this if needed
    /// @brief gets all entities in the active EntityManager
    /// @return m_entities, the std::vector<Entity> (std::vector<std::shared_ptr<Entity>>) of all entities
    std::vector<Entity> &getEntities()
    {
        return m_entities;
    }

    /// TODO: implement the new version of this if needed, also could change this to return only entity IDs to not have to create a bunch of new entities (but consider how that affects usage in scene_play and so on), make this return a reference with data stored in memory if needed
    /// @brief gets all entities with a specified tag
    /// @param tag the type of entity to return (e.g., "player")
    /// @return an std::vector<Entity> of entities with the specified tag
    std::vector<Entity> getEntities(const std::string &tag)
    {
        return EntityMemoryPool::Instance().getEntities(tag);

        // old implementation with entity map
        // if (m_entityMap.find(tag) == m_entityMap.end())
        // {
        //     m_entityMap[tag] = std::vector<Entity>();
        // }
        // return m_entityMap[tag];
    }

    /// TODO: implement the new version of this if needed
    /// @brief gets the map of entity tags to entities
    /// @return m_entityMap, the map of std::string tags to std::vector<Entity> entity vectors
    // const std::map<std::string, std::vector<Entity>> &getEntityMap()
    // {
    //     return m_entityMap;
    // }
};