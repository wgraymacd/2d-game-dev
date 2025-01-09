#pragma once

#include "Components.hpp"

#include <vector>
#include <string>
#include <queue>

class Entity; // forward declaration

// |      |  E0  |  E1  |  E2  |  E3  |  E4  |  E5  |  E6  |  E7  |  E8  |
// |------| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
// |  C1  | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F |
// |  C2  | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F |
// |  C3  | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F |
// | Tag  |  ""  |  ""  |  ""  |  ""  |  ""  |  ""  |  ""  |  ""  |  ""  |
// |Active|  F   |  F   |  F   |  F   |  F   |  F   |  F   |  F   |  F   |

/// TODO: consider creating a separate pool for tiles since they will never have many of the components available to all entities
class EntityMemoryPool
{
    /// TODO: what happens when I reach 10000 entities?
    const static unsigned long MAX_ENTITIES = 9999; // number of columns in the memory pool
    std::queue<unsigned long> m_freeList;           // stores indices of inactive entities to accelerate searching

    unsigned long m_numEntities = 0;
    std::tuple<
        std::vector<CTransform>,
        std::vector<CLifespan>,
        std::vector<CDamage>,
        std::vector<CInvincibility>, // brief moment after taking damage
        std::vector<CHealth>,
        std::vector<CInput>,
        std::vector<CBoundingBox>,
        std::vector<CAnimation>,
        std::vector<CGravity>,
        std::vector<CState>,
        std::vector<CFollowPlayer>, // NPC behavior
        std::vector<CPatrol>        // NPC behavior
        >
        m_pool;
    std::vector<std::string> m_tags;
    std::vector<bool> m_active;

    // initialize free list with all indices
    EntityMemoryPool(unsigned long maxEntities);

    unsigned long getInactiveEntityIndex();

public:
    static EntityMemoryPool &Instance();

    /// TODO: worth implementing here or is this more inneficient than having a separate vector in EntityManager
    // /// @brief get a complete vector of all active entities
    // std::vector<Entity> getEntities();

    // /// @brief get a vector of all entities with the tag tag
    // std::vector<Entity> getEntities(const std::string &tag);

    /// @brief returns a component of type T from an entity with ID entityID
    template <typename T>
    T &getComponent(unsigned long entityID)
    {
        return std::get<std::vector<T>>(m_pool)[entityID];
    }

    /// @brief check to see if entity entityID has a component of type T
    /// TODO: implement this
    template <typename T>
    bool hasComponent(unsigned long entityID)
    {
        return false;
    }

    /// @brief add a component of type T with arguments mArgs of types TArgs to entity entityID
    /// @return the added component
    template <typename T, typename... TArgs>
    T &addComponent(unsigned long entityID, TArgs &&...mArgs)
    {
        T &comp = std::get<std::vector<T>>(m_pool)[entityID];
        comp = T(std::forward<TArgs>(mArgs)...);

        return comp;
    }

    /// @brief get the tag of entity entityID
    const std::string &getTag(unsigned long entityID) const;

    /// @brief add an entity with tag tag
    /// @return the added entity
    Entity addEntity(const std::string &tag);

    /// @brief remove an entity from the memory pool
    void removeEntity(unsigned long entityID);

    /// TODO: return a reference? if I can get entity vectors to be stored in memory longer than duration of this scope
    /// @brief get all entity IDs with the tag tag
    std::vector<Entity> getEntities(const std::string &tag);
};