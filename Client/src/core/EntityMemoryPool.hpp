// Copyright 2025, William MacDonald, All Rights Reserved.

/// NOTE: std::queue, vector, and tuple all manage their own memory, clearing everything when they go out of scope
/// NOTE: static singleton automatically detroyed when program terminates
/// NOTE: if you don't use raw pointer or manual `new` and `delete` calls, then you don't need the destructor

/// TODO: optimize tiles vs other entities better, maybe make a Tile.hpp file with a Tile struct instead of doing the ECS way (if data accessed all together, definitly faster), or edit the way the memory pools are tied together to get two entirely separate ones with separate functions (addComponent redirects to addOtherEntityComponent or addTileComponent depending on entity type which can be checked with entity ID > maxTiles)
/// TODO: mix the pools again for more efficient checks when jumping between tile matrix and other entities like collisions

#pragma once

#include "Components.hpp"
#include "utility/Globals.hpp"

#include <vector>
#include <string>
#include <queue>
#include <iostream>
// #include <unordered_map>

/// container typename method
// template <typename T>
// struct ComponentContainer;
// template <>
// struct ComponentContainer<CTransform> { using Type = std::vector<CTransform>; };
// template <>
// struct ComponentContainer<CAnimation> { using Type = std::vector<CAnimation>; };
// template <>
// struct ComponentContainer<CBoundingBox> { using Type = std::vector<CBoundingBox>; };
// template <>
// struct ComponentContainer<CHealth> { using Type = std::vector<CHealth>; };
// template <>
// struct ComponentContainer<CLifespan> { using Type = std::unordered_map<EntityID, CLifespan>; };
// template <>
// struct ComponentContainer<CDamage> { using Type = std::unordered_map<EntityID, CDamage>; };
// template <>
// struct ComponentContainer<CInvincibility> { using Type = std::unordered_map<EntityID, CInvincibility>; };
// template <>
// struct ComponentContainer<CInput> { using Type = std::unordered_map<EntityID, CInput>; };
// template <>
// struct ComponentContainer<CGravity> { using Type = std::unordered_map<EntityID, CGravity>; };
// template <>
// struct ComponentContainer<CState> { using Type = std::unordered_map<EntityID, CState>; };
// template <>
// struct ComponentContainer<CFireRate> { using Type = std::unordered_map<EntityID, CFireRate>; };
// template <>
// struct ComponentContainer<CFollowPlayer> { using Type = std::unordered_map<EntityID, CFollowPlayer>; };
// template <>
// struct ComponentContainer<CPatrol> { using Type = std::unordered_map<EntityID, CPatrol>; };
// template <typename T>
// using ContainerType = typename ComponentContainer<T>::Type;

class Entity; // forward declaration

// |      |  E0  |  E1  |  E2  |  E3  |  E4  |  E5  |  E6  |  E7  |  E8  |
// |------| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
// |  C1  | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F |
// |  C2  | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F |
// |  C3  | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F | 0, F |
// | Tag  |  ""  |  ""  |  ""  |  ""  |  ""  |  ""  |  ""  |  ""  |  ""  |
// |Active|  F   |  F   |  F   |  F   |  F   |  F   |  F   |  F   |  F   |

class EntityMemoryPool
{
    // EntityID m_maxTiles;
    EntityID m_maxEntities;

    // std::queue<EntityID> m_tileFreeList; // stores indices of inactive tiles to accelerate searching
    std::queue<EntityID> m_entityFreeList; // stores indices of inactive entities to accelerate searching

    // using vectors for frequently needed components and unordered maps for sparse ones
    // tuple stored on stack, vector istelf on stack but elements they hold allocated dynamically on heap
    // std::tuple<
    //     std::vector<CTransform>,
    //     std::vector<CAnimation>,
    //     std::vector<CBoundingBox>,
    //     std::vector<CHealth>,
    //     std::unordered_map<EntityID, CLifespan>,
    //     std::unordered_map<EntityID, CDamage>,
    //     std::unordered_map<EntityID, CInvincibility>, // brief moment after taking damage
    //     std::unordered_map<EntityID, CInput>, // for player only (for now...)
    //     std::unordered_map<EntityID, CGravity>,
    //     std::unordered_map<EntityID, CState>, // "air", "stand", "run"
    //     std::unordered_map<EntityID, CFireRate>,
    //     std::unordered_map<EntityID, CFollowPlayer>, // NPC behavior
    //     std::unordered_map<EntityID, CPatrol> // NPC behavior
    // > m_pool;

    // tiles (and other things that take up one space in the tileMatrix) (layer 0)
    // std::tuple<
    //     std::vector<CType>,
    //     // std::vector<CPosition>,
    //     // std::vector<CBoundingBox>, /// TODO: could change this and health to their own versions for tiles, would simplify the get, has, and add component functions, could even just create a single CTileData component, but that introduce unnecessary accessing for physics vs rendering...
    //     std::vector<CHealth>,
    //     std::vector<CColor>
    //     // std::vector<CGravity> /// TODO: will need this if tiles are falling, and will have to add back CTransform or new CVelocity and CRotation stuff
    // > m_tilePool;

    // all entities but tiles (layer 0) and decorations (layer 1) /// TODO: maybe use unordered_map here instead of vectors if entities are hella spread out component-wise and there are lots of them, wasting memory
    std::tuple<
        std::vector<CAnimation>,
        std::vector<CTransform>,
        std::vector<CBoundingBox>,
        std::vector<CHealth>,
        std::vector<CLifespan>,
        std::vector<CDamage>,
        std::vector<CInvincibility>, // brief moment after taking damage
        std::vector<CInput>, // for player only (for now...)
        std::vector<CGravity>,
        std::vector<CState>, // "air", "stand", "run"
        std::vector<CFire>,
        std::vector<CJointRelation>,
        std::vector<CJointInfo>
        // std::vector<CFollowPlayer>, // NPC behavior
        // std::vector<CPatrol> // NPC behavior
    > m_pool;

    std::vector<bool> m_active;

    // decorations (layer 1) /// TODO: think of where to include these or if use new memory pool

    EntityMemoryPool(EntityID maxEntities);
    // EntityMemoryPool(EntityID maxTiles, EntityID maxEntities);

    // void resetEntityAtIndex(EntityID index);

    /// for the vector and unordered map pool implementation
    // template <typename Container>
    // void resetContainer(EntityID index, Container& container)
    // {
    //     container[index] = typename Container::value_type{};
    //     container.erase(index);
    //     if (typeid(container) == typeid(std::vector<typename Container::value_type>)) {
    //         auto& vec = static_cast<std::vector<typename Container::value_type>&>(container);
    //         if (index < vec.size()) {
    //             vec[index] = typename Container::value_type{};
    //         }
    //     }
    //     else if (typeid(container) == typeid(std::unordered_map<EntityID, typename Container::mapped_type>)) {
    //         auto& map = static_cast<std::unordered_map<EntityID, typename Container::mapped_type>&>(container);
    //         map.erase(index);
    //     }
    //     else {
    //         throw std::runtime_error("Unsupported container type in resetContainer.");
    //     }
    //     if constexpr (std::is_same_v<Container, std::unordered_map<EntityID, typename std::decay_t<Container>::mapped_type>>) {
    //         // Handle std::unordered_map
    //         container.erase(index);
    //     }
    //     else if constexpr (std::is_same_v<Container, std::vector<typename Container::value_type>>) {
    //         // Handle std::vector
    //         if (index < container.size()) {
    //             container[index] = typename Container::value_type{};
    //         }
    //     }
    //     else {
    //         // Unsupported container type
    //         throw std::runtime_error("Unsupported container type in resetContainer.");
    //     }
    // }

    /// @brief helper for static assertions on vector retrievals in memory pools, compile time
    // template <typename T>
    // static constexpr bool isTileComponent()
    // {
    //     return std::is_same_v<T, CType> ||
    //         std::is_same_v<T, CHealth> ||
    //         std::is_same_v<T, CColor>;
    // }

public:
    static EntityMemoryPool& Instance();

    /// @brief returns a component of type T from an entity with ID entityID
    /// TODO: may be better way to separate tiles and other entities than doing this if else (same for methods below)
    template <typename T>
    T& getComponent(EntityID entityID)
    {
        // if constexpr (!isTileComponent<T>())
        // {
        return std::get<std::vector<T>>(m_pool)[entityID];
        // return std::get<std::vector<T>>(m_otherEntityPool)[entityID - m_maxTiles];
    // }
    // else
    // {
    //     if (entityID < m_maxTiles) // entity is a tile
    //     {
    //         return std::get<std::vector<T>>(m_tilePool)[entityID];
    //     }
    //     else // entity is something else
    //     {
    //         return std::get<std::vector<T>>(m_otherEntityPool)[entityID - m_maxTiles];
    //     }
    // }

    /// runtime check for vector vs map
    // if (std::is_same_v<T, CTransform> || std::is_same_v<T, CAnimation> || std::is_same_v<T, CBoundingBox> || std::is_same_v<T, CHealth>)
    // {
    //     return std::get<std::vector<T>>(m_pool)[entityID];
    // }
    // else
    // {
    //     return std::get<std::unordered_map<EntityID, T>>(m_pool)[entityID]; /// TODO: these are what causes the error, just the line being here
    // }

    /// container typename method
    // auto& container = std::get<ContainerType<T>>(m_pool);
    // return container[entityID];
    }

    /// @brief check to see if entity entityID has a component of type T
    /// TODO: implement this
    template <typename T>
    bool hasComponent(EntityID entityID)
    {
        // if constexpr (!isTileComponent<T>())
        // {
        std::vector<T>& componentVector = std::get<std::vector<T>>(m_pool);
        T& component = componentVector[entityID];
        // T& component = componentVector[entityID - m_maxTiles];
        return component.exists;
        // }
        // else
        // {
        //     if (entityID < m_maxTiles) // entity is a tile
        //     {
        //         std::vector<T>& componentVector = std::get<std::vector<T>>(m_tilePool);
        //         T& component = componentVector[entityID];
        //         return component.exists;
        //     }
        //     else // entity is something else
        //     {
        //         std::vector<T>& componentVector = std::get<std::vector<T>>(m_otherEntityPool);
        //         T& component = componentVector[entityID - m_maxTiles];
        //         return component.exists;
        //     }
        // }

        /// runtime check for vector vs map
        // if (std::is_same_v<T, CTransform> || std::is_same_v<T, CAnimation> || std::is_same_v<T, CBoundingBox> || std::is_same_v<T, CHealth>)
        // {
        //     std::vector<T>& componentVector = std::get<std::vector<T>>(m_pool);
        //     const T& component = componentVector[entityID];
        //     return component.exists;
        // }
        // else
        // {
        //     std::unordered_map<EntityID, T>& componentMap = std::get<std::unordered_map<EntityID, T>>(m_pool);
        //     const T& component = componentMap[entityID];
        //     return component.exists;
        // }

        /// container typename method
        // auto& container = std::get<ContainerType<T>>(m_pool);
        // const T& component = container[entityID];
        // return component.exists;
    }

    /// @brief add a component of type T with arguments mArgs of types TArgs to entity entityID
    /// @return the added component
    template <typename T, typename... TArgs>
    T& addComponent(EntityID entityID, TArgs &&...mArgs)
    {
        // std::cout << "adding component to entity " << entityID << std::endl;
        // if constexpr (!isTileComponent<T>()) // not a tile component
        // {
        std::vector<T>& componentVector = std::get<std::vector<T>>(m_pool);
        T& component = componentVector[entityID];
        // T& component = componentVector[entityID - m_maxTiles];
        component = T(std::forward<TArgs>(mArgs)...);
        component.exists = true;
        return component;
        // }
        // else // could be tile or other entity
        // {
        //     if (entityID < m_maxTiles) // entity is a tile
        //     {
        //         std::vector<T>& componentVector = std::get<std::vector<T>>(m_tilePool);
        //         T& component = componentVector[entityID];
        //         component = T(std::forward<TArgs>(mArgs)...);
        //         component.exists = true;
        //         return component;
        //     }
        //     else // entity is something else
        //     {
        //         std::vector<T>& componentVector = std::get<std::vector<T>>(m_otherEntityPool);
        //         T& component = componentVector[entityID - m_maxTiles];
        //         component = T(std::forward<TArgs>(mArgs)...);
        //         component.exists = true;
        //         return component;
        //     }
        // }

        /// runtime check for vector vs map
        // if (std::is_same_v<T, CTransform> || std::is_same_v<T, CAnimation> || std::is_same_v<T, CBoundingBox> || std::is_same_v<T, CHealth>)
        // {
        //     std::vector<T>& componentVector = std::get<std::vector<T>>(m_pool);
        //     T& comp = componentVector[entityID];
        //     comp = T(std::forward<TArgs>(mArgs)...);
        //     comp.exists = true;
        //     return comp;
        // }
        // else
        // {
        //     std::unordered_map<EntityID, T>& componentMap = std::get<std::unordered_map<EntityID, T>>(m_pool);
        //     T& comp = componentMap[entityID];
        //     comp = T(std::forward<TArgs>(mArgs)...);
        //     comp.exists = true;
        //     return comp;
        // }

        /// container typename method
        // auto& container = std::get<ContainerType<T>>(m_pool);
        // // const T& component = container[entityID];
        // // component = T(std::forward<TArgs>(mArgs)...);
        // // component.exists = true;
        // // return component;
        // container[entityID] = T(std::forward<TArgs>(mArgs)...);
        // container[entityID].exists = true;
        // return container[entityID];
    }

    /// @brief add an entity with tag tag
    /// @return the added entity
    Entity addEntity(const std::string& tag);

    /// @brief remove an entity from the memory pool
    void removeEntity(EntityID entityID);

    /// @brief return a bool representing the entity's living status
    bool isActive(EntityID entityID);
};