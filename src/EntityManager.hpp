#pragma once

#include "Entity.hpp"
#include "EntityMemoryPool.hpp"
#include "Globals.hpp"

#include "Timer.hpp"

#include <string>
#include <vector>

class EntityManager
{
    // tile map
    Vec2ui m_worldSizeCells; // size of the world in grid units
    Vec2ui m_cellSizePixels; // size of one cell in pixels
    std::vector<std::vector<Entity>> m_tileMatrix; // matrix[x][y] = tile at grid pos (x, y), initialized in constructor
    /// TODO: could change tileMatrix to 1D array for even better chache performance when iterating over all tiles
    /// TODO: also consider chunk-based (map of pairs to chunks and chunks are 1d flat arrays of tiles) or quadtree storage, both are more efficient memory usage for more sparse worlds, efficient neighbor access, dynamic world size, and can still check neighbors with the indices regrdless

    std::vector<std::pair<std::string, Entity>> m_entitiesToAdd;

    // implementation with entity map (as opposed to searching for all entities with a given tag in the memory pool)
    std::map<std::string, std::vector<Entity>> m_entityMap; // collidable layer entities without a dedicated layer matrix (player, bullet, weapon, npc, etc.), NO TILES

    /// @brief remove dead entities from param entities
    void removeDeadEntities()
    {
        PROFILE_FUNCTION();

        // remove from m_liveEntities
        // auto it = std::remove_if(m_liveEntities.begin(), m_liveEntities.end(),
        //     [](const Entity& entity) { return !entity.isActive(); });
        // m_liveEntities.erase(it, m_liveEntities.end());

        // remove from m_entityMap
        /// TODO: super slow if m_entityMap becomes large
        /// TODO: try the locational thing to the player like the tileMatrix[x][y]? try pre initialized vectors and just use active vs inactive? remove map entirely in favor of different memory pools or something for each type of entity?
        for (auto& [key, entityVector] : m_entityMap)
        {
            auto itMap = std::remove_if(entityVector.begin(), entityVector.end(),
                [](const Entity& entity) { return !entity.isActive(); });
            entityVector.erase(itMap, entityVector.end());
        }
    }

public:
    EntityManager(const Vec2ui& worldSize, const Vec2ui& cellSizePixels) : m_worldSizeCells(worldSize), m_cellSizePixels(cellSizePixels), m_tileMatrix(m_worldSizeCells.x, std::vector<Entity>(m_worldSizeCells.y)) {}

    /// TODO: implement new version of this if needed
    /// @brief adds entities to be added and removes entities to be destroyed
    void update()
    {
        PROFILE_FUNCTION();

        for (std::pair<std::string, Entity>& pair : m_entitiesToAdd)
        {
            m_entityMap[pair.first].push_back(pair.second);
        }
        m_entitiesToAdd.clear();

        removeDeadEntities();
    }

    /// @brief marks new entity to be added on next call to EntityManager::update
    /// @param tag the type of entity to add (e.g., "player")
    /// @return the entity to be added
    Entity addEntity(const std::string& tag)
    {
        // PROFILE_FUNCTION();

        Entity e = EntityMemoryPool::Instance().addEntity(tag);
        if (tag == "player" || tag == "bullet" || tag == "weapon")
        {
            m_entitiesToAdd.push_back(std::pair<std::string, Entity>(tag, e));
        }
        return e;
    }

    /// TODO: implement the new version of this if needed
    /// @brief gets all entities in the active EntityManager
    /// @return m_liveEntities, the std::vector<Entity> (std::vector<std::shared_ptr<Entity>>) of all entities
    // std::vector<Entity>& getEntities()
    // {
    //     return m_liveEntities;
    // }

    std::vector<std::vector<Entity>>& getTileMatrix()
    {
        return m_tileMatrix;
    }

    /// TODO: this may not be the best way, but I'm testing, it's getting messy
    void addTileToMatrix(Entity& tile, const int x, const int y)
    {
        PROFILE_FUNCTION();

        // const Vec2f& pos = tile.getComponent<CPosition>().pos;
        m_tileMatrix[x][y] = tile;
    }

    /// TODO: implement the new version of this if needed, also could change this to return only entity IDs to not have to create a bunch of new entities (but consider how that affects usage in ScenePlay and so on), make this return a reference with data stored in memory if needed
    /// @brief gets all entities with a specified tag
    /// @param tag the type of entity to return (e.g., "player")
    /// @return an std::vector<Entity> of entities with the specified tag
    std::vector<Entity>& getEntities(const std::string& tag)
    {
        PROFILE_FUNCTION();

        // implementation with entity map
        if (m_entityMap.find(tag) == m_entityMap.end())
        {
            m_entityMap[tag] = std::vector<Entity>();
        }
        return m_entityMap[tag];
    }

    /// TODO: implement the new version of this if needed
    /// @brief gets the map of entity tags to entities
    /// @return m_entityMap, the map of std::string tags to std::vector<Entity> entity vectors
    const std::map<std::string, std::vector<Entity>>& getEntityMap()
    {
        return m_entityMap;
    }
};