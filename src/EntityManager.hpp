#pragma once

#include "Entity.hpp"
#include <map>
#include <string>
#include <vector>
#include <memory> // shared pointers

using EntityVec = std::vector<std::shared_ptr<Entity>>;

class EntityManager
{
    EntityVec m_entities;
    EntityVec m_entitiesToAdd;
    std::map<std::string, EntityVec> m_entityMap;
    size_t m_totalEntities = 0;

    void removeDeadEntities(EntityVec &vec)
    {
        vec.erase(
            std::remove_if(vec.begin(), vec.end(),
                [](const std::shared_ptr<Entity> &e)
                {
                    return !e->m_active;
                }),
            vec.end());
    }

public:

    EntityManager() = default;

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
            m_entityMap[e->tag()].push_back(e);
        }
        m_entitiesToAdd.clear();

        removeDeadEntities(m_entities);

        for (auto &[tag, entityVec] : m_entityMap)
        {
            removeDeadEntities(entityVec);
        }

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

    // add entity
    std::shared_ptr<Entity> addEntity(const std::string& tag)
    {
        std::shared_ptr<Entity> entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

        m_entitiesToAdd.push_back(entity);

        if (m_entityMap.find(tag) == m_entityMap.end())
        {
            m_entityMap[tag] = EntityVec();
        }

        return entity;
    }

    // return all entities
    const EntityVec &getEntities()
    {
        return m_entities;
    }

    // return entities with specific tag
    const EntityVec &getEntities(const std::string &tag)
    {
        if (m_entityMap.find(tag) == m_entityMap.end())
        {
            m_entityMap[tag] = EntityVec();
        }
        return m_entityMap[tag];
    }

    // return the map of entity string tags to entities
    const std::map<std::string, EntityVec> &getEntityMap()
    {
        return m_entityMap;
    }
};