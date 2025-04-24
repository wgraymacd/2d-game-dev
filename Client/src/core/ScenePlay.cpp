// Copyright 2025, William MacDonald, All Rights Reserved.

/// TODO: multithreading
/// TODO: worry about signed vs unsigned and size and even int (adapts to platform's word size) and all that later after learning more about performance differences and such (e.g., may not want to mix signed and unsigned ints like uint32_t and int8_t)
/// TODO: in some cases processing a 64-bit int is faster than a 32-bit one (or 32 faster than 16), but in many cases memory is what slows down a program, so just have to test between memory efficiency and CPU efficiency
/// TODO: render tile layer (and any other things) at the minimum resolution in it's own view, then scale the size of that view to match with others (this way the resolution of the tile map can be shit (and minimap) but still look the same, and the character and guns and all can be great, can even put back in textures for tiles probably)

// Core
#include "ScenePlay.hpp"
#include "Scene.hpp"
#include "GameEngine.hpp"
#include "EntityManager.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include "Animation.hpp"
#include "Action.hpp"

// Physics
#include "physics/Vec2.hpp"
#include "physics/Physics.hpp"

// World
#include "world/WorldGenerator.hpp"
#include "world/TileType.hpp"

// Global
#include "Random.hpp"
#include "Timer.hpp"

// External libraries
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

// C++ standard libraries
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <chrono>
#include <unordered_set>

/// @param gameEngine the game's main engine which handles scene switching and adding, and other top-level functions; required by Scene to set m_game
ScenePlay::ScenePlay(GameEngine& gameEngine, int worldSeed) : Scene(gameEngine)
{
    init();
    loadGame();
    generateWorld(worldSeed);
    spawnPlayer();
}

/**
 * scene initialization functions
 */

 /// @brief registers keybinds, sets fps text attributes, minimap viewport, and calls loadGame()
void ScenePlay::init()
{
    PROFILE_FUNCTION();

    // misc keybind setup
    registerAction(static_cast<unsigned int>(sf::Keyboard::Key::P), "PAUSE");
    registerAction(static_cast<unsigned int>(sf::Keyboard::Key::Escape), "QUIT"); /// TODO: change to show HUD or menu or something without leaving the game
    registerAction(static_cast<unsigned int>(sf::Keyboard::Key::M), "TOGGLE_MAP");
    // player keyboard setup
    registerAction(static_cast<unsigned int>(sf::Keyboard::Key::W), "JUMP");
    registerAction(static_cast<unsigned int>(sf::Keyboard::Key::A), "LEFT");
    registerAction(static_cast<unsigned int>(sf::Keyboard::Key::D), "RIGHT");
    // player mouse setup
    registerAction(static_cast<unsigned int>(sf::Mouse::Button::Left), "SHOOT", true);

    // fps counter setup
    m_fpsText.setCharacterSize(12);
    m_fpsText.setFillColor(sf::Color::White);
    m_fpsText.setPosition({ 10.f, 10.f }); // top-left corner

    // minimap setup
    m_miniMapView.setViewport(sf::FloatRect({ 0.75f, 0.1f }, { 0.2f, 0.3556f })); /// TODO: customizable size, viewport, etc.
}

/// @brief loads data that is individual to the specific player, generates world, spawns player
void ScenePlay::loadGame()
{
    PROFILE_FUNCTION();

    // read in the level file and add the appropriate entities
    std::ifstream file("bin/playerConfig.txt");

    if (!file.is_open())
    {
        std::cerr << "Player file could not be opened: " << "bin/playerConfig.txt" << std::endl;
        exit(-1);
    }

    std::string type;

    while (file >> type)
    {
        if (type == "Player")
        {
            file >> m_playerConfig.CW >> m_playerConfig.CH >> m_playerConfig.SX >> m_playerConfig.SY >> m_playerConfig.SM >> m_playerConfig.GRAVITY >> m_playerConfig.BA;
        }
        else
        {
            std::cerr << "Type not allowed: " << type << std::endl;
            exit(-1);
        }
    }
    file.close();
}

/// @brief randomly generate the playing world
void ScenePlay::generateWorld(int worldSeed)
{
    PROFILE_FUNCTION();

    /// TODO: mountains, caves, lakes, rivers, even terrain, biomes, etc.

    /*
        - example:
        - first pass: top 1/3 dirt, bottom 2/3 stone
        - second pass: dirt vains in stone and stone blobs in dirt
        - add in ores (function of depth)
        - add in caves (prolly function of depth)
        - add a single deep cave connnecting to surface and going deep at some random location
        - cut out skyline using noise (Perlin prolly)
        - add in other things like houses, dungeon, decorations, etc.
        - grow grass on surface, use code to spread grass and grow vines and things
        - grow trees
        - reskin tiles process to make them all fit together nicely
        - fill in liquids
        - illumiate everything, pretty it up, create edge vector if needed for polygon stuff and new ray casting (updated on changes thereafter)
    */

    // Generate world and get tile positions
    WorldGenerator gen(m_worldMaxCells.x, m_worldMaxCells.y, worldSeed);
    gen.generateWorld();
    const std::vector<TileType>& tileTypes = gen.getTileTypes();

    // Spawn tiles according to their positions in the grid
    for (size_t x = 0; x < m_worldMaxCells.x; ++x)
    {
        // PROFILE_SCOPE("adding tiles, row x");

        for (size_t y = 0; y < m_worldMaxCells.y; ++y)
        {
            TileType tileType = tileTypes[x * m_worldMaxCells.y + y];
            if (tileType != TileType::NONE)
            {
                // std::cout << "adding tile " << x << ", " << y << std::endl;

                Tile tile;
                tile.type = tileType;
                tile.light = 0; /// TODO: instead of propagating light, just make all tiles darker than usual, then render the ones in sight brighter?

                // float randomNumber = Random::getFloatingPoint(0.9f, 1.1f);
                uint8_t min = 0;
                uint8_t max = 5;
                uint8_t randomNumber = Random::getIntegral(min, max); /// TODO: could be faster to use predetermined values or formula, like pseudo-random function of depth or something

                if (tileType == TileType::DIRT)
                {
                    tile.health = 40;
                    // tile.r = static_cast<uint8_t>(50.0f * randomNumber);
                    // tile.g = static_cast<uint8_t>(30.0f * randomNumber);
                    // tile.b = static_cast<uint8_t>(20.0f * randomNumber);
                    tile.r = 50 + randomNumber;
                    tile.g = 30 + randomNumber;
                    tile.b = 20 + randomNumber;
                    tile.blocksMovement = true;
                    tile.blocksVision = true;
                }
                else if (tileType == TileType::DIRTWALL)
                {
                    tile.health = 10;
                    tile.r = 25 + randomNumber;
                    tile.g = 15 + randomNumber;
                    tile.b = 10 + randomNumber;
                    // tile.r = static_cast<uint8_t>(25.0f * randomNumber);
                    // tile.g = static_cast<uint8_t>(15.0f * randomNumber);
                    // tile.b = static_cast<uint8_t>(10.0f * randomNumber);
                    tile.blocksMovement = false;
                    tile.blocksVision = false;
                }
                else if (tileType == TileType::STONE)
                {
                    tile.health = 60;
                    // tile.r = static_cast<uint8_t>(70.0f * randomNumber);
                    // tile.g = static_cast<uint8_t>(70.0f * randomNumber);
                    // tile.b = static_cast<uint8_t>(70.0f * randomNumber);
                    tile.r = 70 + randomNumber;
                    tile.g = 70 + randomNumber;
                    tile.b = 70 + randomNumber;
                    tile.blocksMovement = true;
                    tile.blocksVision = true;
                }
                else if (tileType == TileType::STONEWALL)
                {
                    tile.health = 10;
                    // tile.r = static_cast<uint8_t>(35.0f * randomNumber);
                    // tile.g = static_cast<uint8_t>(35.0f * randomNumber);
                    // tile.b = static_cast<uint8_t>(35.0f * randomNumber);
                    tile.r = 35 + randomNumber;
                    tile.g = 35 + randomNumber;
                    tile.b = 35 + randomNumber;
                    tile.blocksMovement = false;
                    tile.blocksVision = false;
                }
                else if (tileType == TileType::BRICK)
                {
                    tile.health = 100;
                    // tile.r = static_cast<uint8_t>(50.0f * randomNumber);
                    // tile.g = static_cast<uint8_t>(0.0f);
                    // tile.b = static_cast<uint8_t>(0.0f);
                    tile.r = 50 + randomNumber;
                    tile.g = 0;
                    tile.b = 0;
                    tile.blocksMovement = true;
                    tile.blocksVision = true;
                }
                else if (tileType == TileType::BRICKWALL)
                {
                    tile.health = 10;
                    // tile.r = static_cast<uint8_t>(25.0f * randomNumber);
                    // tile.g = static_cast<uint8_t>(0.0f);
                    // tile.b = static_cast<uint8_t>(0.0f);
                    tile.r = 25 + randomNumber;
                    tile.g = 0;
                    tile.b = 0;
                    tile.blocksMovement = false;
                    tile.blocksVision = false;
                }
                else
                {
                    std::cerr << "invalid tile type: " << tileType << "\n";
                    exit(-1);
                }

                m_tileManager.addTile(tile, x, y);
            }
        }
    }
}

/**
 * scene management functions
 */

 /// @brief update the scene; this function is called by the game engine at each frame if this scene is active
// void ScenePlay::updateState(std::chrono::duration<long long, std::nano>& lag) {
void ScenePlay::updateState()
{
    PROFILE_FUNCTION();

    if (!m_paused)
    {
        // while (lag >= std::chrono::duration<long long, std::nano>(1000000000 / GlobalSettings::frameRate)) /// TODO: consider using doubles or something to be more precise with timing, or just longs to be smaller in memory
        // {
            // this can be infinite loop if it takes longer to do all this than the time per frame
            /// TODO: think about order here if it even matters
        m_game.getNetManager().update(); // update network data /// TODO: may want to move this to the top/bottom of each function if data isn't arriving in time or something

        sStatus(); // lifespan and invincibility time calculations first to not waste calculations on dead entities
        sObjectMovement(); // object movement
        sObjectCollision(); // then object collisions
        sProjectiles(); // then iterations of projectile movement and collisions, then projectile spawns
        sAI();
        sAnimation(); // update all animations (could move this around)
        sCamera(); // finally, set camera

        m_entityManager.update(); // add and remove all entities staged during updates above

        // lag -= std::chrono::duration<long long, std::nano>(1000000000 / GlobalSettings::frameRate); /// TODO: will rounding be an issue here?
    // }
    }

    sRender();
}

/// @brief changes back to MENU scene when this scene ends
void ScenePlay::onEnd()
{
    PROFILE_FUNCTION();

    m_game.changeScene("MENU");

    /// TODO: stop music, play menu music
}

// void ScenePlay::resizeView(const Vec2f& size)
// {
//     m_mainView.setSize(size);
// }

/**
 * systems
 */

 /// @brief handle player, weapon, etc. movement per frame before bullet movement/collision (object = non-projectile); includes CTransform, CInput, CState
void ScenePlay::sObjectMovement()
{
    PROFILE_FUNCTION();

    /// perform updates from network
    /// TODO: consider creating separate vectors for separate data types so that they can be updated at the same time as other entities - max cache locality

    // const std::vector<NetworkData>& netData = m_game.getNetManager().getData();

    // for (const NetworkData& netDatum : netData)
    // {
    //     EntityID localID;
    //     Entity entity;

    //     std::cout << netDatum << "\n";

    //     switch (netDatum.dataType)
    //     {
    //         case NetworkData::DataType::POSITION:
    //             localID = m_game.getNetManager().getLocalID(netDatum.first.id);
    //             entity = m_entityManager.getEntity(localID);

    //             /// TODO: update entity's position
    //             break;
    //         case NetworkData::DataType::VELOCITY:
    //             localID = m_game.getNetManager().getLocalID(netDatum.first.id);
    //             entity = m_entityManager.getEntity(localID);

    //             /// TODO: update entity's velocity
    //             break;
    //         case NetworkData::DataType::SPAWN:
    //             /// TODO: this
    //             // Entity entity = m_entityManager.addEntity("CHANGE THIS");
    //             // m_game.getNetManager().updateIDMaps(netDatum.netID, entity.getID());
    //             break;
    //         case NetworkData::DataType::LOCAL_SPAWN:
    //             m_game.getNetManager().updateIDMaps(netDatum.first.id, netDatum.second.id);
    //             break;

    //         default:
    //             std::cerr << "Invalid data type received\n";
    //     }
    // }


    /// perform local updates

    float airResistance = 15.0f; // m/s slow-down

    // local player
    if (m_player.isActive())
    {
        CState& playerState = m_player.getComponent<CState>();
        CInput& playerInput = m_player.getComponent<CInput>();
        CTransform& playerTrans = m_player.getComponent<CTransform>();
        CGravity& playerGrav = m_player.getComponent<CGravity>();
        CTransform& weaponTrans = m_weapon.getComponent<CTransform>();
        CBoundingBox& weaponBox = m_weapon.getComponent<CBoundingBox>();

        Vec2f velToAdd(0.0f, 0.0f);

        /// TODO: consider turing all this into real physics

        if (playerTrans.velocity.y + playerGrav.gravity >= airResistance)
        {
            velToAdd.y += airResistance - playerTrans.velocity.y;
        }
        else
        {
            velToAdd.y += playerGrav.gravity;
        }

        // no left or right input - slow down in x-direction (less if in air, more if on ground) until stopped
        if (!playerInput.left && !playerInput.right)
        {
            // set friction value based on state
            float friction;
            if (playerState.state == State::AIR)
            {
                friction = 0.2f;
            }
            else
            {
                friction = 1.0f;
            }

            // slow down until stopped
            if (abs(playerTrans.velocity.x) >= friction)
            {
                velToAdd.x += (playerTrans.velocity.x > 0 ? -friction : friction);
            }
            else
            {
                velToAdd.x -= playerTrans.velocity.x;
            }
        }

        // move right until reaching max speed and face the direction we are moving in
        if (playerInput.right)
        {
            if (playerTrans.velocity.x + m_playerConfig.SX <= m_playerConfig.SM)
            {
                velToAdd.x += m_playerConfig.SX;
            }
            else
            {
                velToAdd.x = m_playerConfig.SM - playerTrans.velocity.x;
            }

            // playerTrans.scale.x = abs(playerTrans.scale.x);
        }

        if (playerInput.left)
        {
            if (playerTrans.velocity.x - m_playerConfig.SX >= -m_playerConfig.SM)
            {
                velToAdd.x -= m_playerConfig.SX;
            }
            else
            {
                velToAdd.x = -m_playerConfig.SM - playerTrans.velocity.x;
            }

            // playerTrans.scale.x = -abs(playerTrans.scale.x);
        }

        /// TODO: better jumping and flying, min jump height, max jump height if up input held down, consider gravity changing
        if (playerInput.up)
        {
            if (playerState.state == State::AIR)
            {
                // std::cout << "air jump\n";
                velToAdd.y -= 2.0f * m_playerConfig.GRAVITY;
            }
            else // on the ground
            {
                // std::cout << "jumping with state " << playerState.state << "\n";
                velToAdd.y -= m_playerConfig.SY;
            }
        }

        playerTrans.velocity += velToAdd;
        playerTrans.prevPos = playerTrans.pos;
        playerTrans.pos += playerTrans.velocity;

        /// send network updates to server
        // if (playerTrans.prevPos != playerTrans.pos)
        // {
        //     NetworkData netData {
        //         .dataType = NetworkData::DataType::POSITION,
        //         .first.id = m_game.getNetManager().getNetID(m_player.getID()),
        //         .second.f = playerTrans.pos.x,
        //         .third.f = playerTrans.pos.y
        //     };
        //     m_game.getNetManager().sendData(netData);
        // }

        /// TODO: have crouching? does this then go in sUserInput?
        // if (playerTrans.velocity.x == 0 && playerTrans.velocity.y == 0)
        // {
        //     if (playerInput.down)
        //     {
        //         playerState = "crouch";
        //     }
        //     else
        //     {
        //         playerState = "stand";
        //     }
        // }

        // player head and arms
        CTransform& headTrans = m_playerHead.getComponent<CTransform>();
        headTrans.pos.y = playerTrans.pos.y - 30.0f;
        CTransform& backArmTrans = m_playerArmBack.getComponent<CTransform>();
        backArmTrans.pos.y = playerTrans.pos.y - 20.0f;
        CTransform& frontArmTrans = m_playerArmFront.getComponent<CTransform>();
        frontArmTrans.pos.y = playerTrans.pos.y - 25.0f;

        // weapon held by player
        weaponTrans.pos.x = playerTrans.pos.x;
        weaponTrans.pos.y = playerTrans.pos.y - 20.0f;
        const sf::Vector2f& worldTarget = m_game.window().mapPixelToCoords(sf::Mouse::getPosition(m_game.window()));
        const Vec2f aimVec(worldTarget.x - weaponTrans.pos.x, worldTarget.y - weaponTrans.pos.y);
        weaponTrans.pos += aimVec.norm() * weaponBox.halfSize.x / 2.0f;
        weaponTrans.angle = aimVec.angle();

        // set scale based on mouse position
        if (aimVec.x < 0)
        {
            playerTrans.scale.x = -abs(playerTrans.scale.x);
            weaponTrans.scale.y = -abs(weaponTrans.scale.x);
        }
        else
        {
            playerTrans.scale.x = abs(playerTrans.scale.x);
            weaponTrans.scale.y = abs(weaponTrans.scale.x);
        }
        for (Entity& part : m_entityManager.getEntities("playerPart"))
        {
            CTransform& partTrans = part.getComponent<CTransform>();

            partTrans.pos.x = playerTrans.pos.x;
            partTrans.angle = aimVec.angle();

            // set scale based on mouse position
            if (aimVec.x < 0)
            {
                partTrans.scale.y = -abs(partTrans.scale.y);
            }
            else
            {
                partTrans.scale.y = abs(partTrans.scale.y);
            }
        }
    }

    // ragdolls
    for (Entity& ragA : m_entityManager.getEntities("ragdoll"))
    {
        CTransform& ragATrans = ragA.getComponent<CTransform>();
        CBoundingBox& ragABox = ragA.getComponent<CBoundingBox>();
        CGravity& ragAGrav = ragA.getComponent<CGravity>();

        if (ragATrans.velocity.y + ragAGrav.gravity >= airResistance)
        {
            ragATrans.velocity.y += airResistance - ragATrans.velocity.y;
        }
        else
        {
            ragATrans.velocity.y += ragAGrav.gravity;
        }

        // slow down rotation speed over time
        // if (abs(ragATrans.angularVelocity) >= 0.001)
        // {
        //     ragATrans.angularVelocity += (ragATrans.angularVelocity > 0 ? -0.001f : 0.001f);
        // }
        // else
        // {
        //     ragATrans.angularVelocity -= ragATrans.angularVelocity * 0.001f;
        // }

        ragATrans.prevPos = ragATrans.pos;
        ragATrans.pos += ragATrans.velocity / 5.0f;
        ragATrans.prevAngle = ragATrans.angle;
        ragATrans.angle += ragATrans.angularVelocity;

        /// TODO: do the physics force entity thing here, not just the translation and rotation hard coded fix
        if (ragA.hasComponent<CJointRelation>())
        {
            std::cout << "\n\nragA pos: " << ragATrans.pos << "\n";
            std::cout << "ragA vel: " << ragATrans.velocity << "\n";
            std::cout << "ragA ang: " << ragATrans.angle << "\n";
            std::cout << "ragA angVel: " << ragATrans.angularVelocity << "\n";

            CJointRelation& joint = ragA.getComponent<CJointRelation>();

            CJointInfo& ragAInfo = ragA.getComponent<CJointInfo>();

            const Entity& ragB = m_entityManager.getEntity(joint.entityID);
            CTransform& ragBTrans = ragB.getComponent<CTransform>();
            CBoundingBox& ragBBox = ragB.getComponent<CBoundingBox>();
            CJointInfo& ragBInfo = ragB.getComponent<CJointInfo>();

            std::cout << "ragB pos: " << ragBTrans.pos << std::endl;
            std::cout << "ragB vel: " << ragBTrans.velocity << std::endl;
            std::cout << "ragB ang: " << ragBTrans.angle << "\n";
            std::cout << "ragB angVel: " << ragBTrans.angularVelocity << "\n";

            float angleDiff = ragBTrans.angle - ragATrans.angle;
            float angleError = 0.0f;
            if (angleDiff < joint.minAngle)
            {
                angleError = (angleDiff - joint.minAngle) / 2.0f; // < 0
            }
            else if (angleDiff > joint.maxAngle)
            {
                angleError = (angleDiff - joint.maxAngle) / 2.0f; // > 0
            }

            std::cout << "angleDiff: " << angleDiff << "\n";
            std::cout << "angleError: " << angleError << "\n";

            for (size_t i = 2; i >= 0; --i) // 3 is the size of the joint positions array
            {
                std::cout << "connecting rag A to ragB with pos " << i << std::endl;
                float jointAOffset = ragAInfo.initJointOffsets[i];
                if (jointAOffset != 0.0f) // found a joint pos /// TODO: may want to add some theshold or something
                {
                    float jointBOffset = ragBInfo.initJointOffsets[i]; // must be a joint in rag B joint info at same index

                    Vec2f ragAJointPos = ragATrans.pos + Vec2f(0.0f, jointAOffset).rotate(ragATrans.angle);
                    Vec2f ragBJointPos = ragBTrans.pos + Vec2f(0.0f, jointBOffset).rotate(ragBTrans.angle);

                    if (abs(angleDiff) > 0.0001f)
                    {
                        // restrict angle
                        // ragATrans.angle += angleError;
                        // ragBTrans.angle -= angleError;
                        ragATrans.angularVelocity += angleError * 0.01f;
                        ragBTrans.angularVelocity -= angleError * 0.01f;
                    }

                    std::cout << "new ragA ang: " << ragATrans.angle << "\n";
                    std::cout << "new ragA angVel: " << ragATrans.angularVelocity << "\n";
                    std::cout << "new ragB ang: " << ragBTrans.angle << "\n";
                    std::cout << "new ragB angVel: " << ragBTrans.angularVelocity << "\n";

                    Vec2f diff = ragBJointPos - ragAJointPos;
                    float dist = diff.length();

                    std::cout << "diff: " << diff << "\n";
                    std::cout << "dist: " << dist << "\n";

                    if (dist > 0.0001f)
                    {
                        Vec2f correction = diff / 2.0f; /// TODO: by correcting like this (I think), I am cutting the effects of gravity in half

                        // put joints back together
                        ragATrans.pos += correction;
                        ragBTrans.pos -= correction;

                        // apply equal and opposite forces on each joint
                        Physics::ForceEntity(ragATrans.pos, ragATrans.velocity, ragATrans.angularVelocity, ragABox.size, correction / 10.0f, ragAJointPos);
                        Physics::ForceEntity(ragBTrans.pos, ragBTrans.velocity, ragBTrans.angularVelocity, ragBBox.size, -correction / 10.0f, ragBJointPos);
                    }

                    break;
                }
            }

            std::cout << "new ragA pos: " << ragATrans.pos << std::endl;
            std::cout << "new ragB pos: " << ragBTrans.pos << std::endl;
        }
    }
}

/// TODO: modularize some of this if needed to reduce repition and make it easier to read
/// TODO: increase efficiency with chunking or something like that, maybe a distance check or an in-frame/in-window check if possible
/// @brief handle collisions and m_player CState updates; includes tile matrix, CTransform, CState, CBoundingBox, CInput
void ScenePlay::sObjectCollision()
{
    PROFILE_FUNCTION();

    // cache once per frame
    const std::vector<Tile>& tiles = m_tileManager.getTiles();

    playerTileCollisions(tiles);

    /// TODO: weapon-tile collisions (like pistol that fell out of someones hand when killed), other object collisions

    // ragdoll-tile collisions /// TODO: could just do two vertices on a stick and call it a day (or give the vertices a circular distance for collisions)
    for (Entity& rag : m_entityManager.getEntities("ragdoll"))
    {
        CTransform& trans = rag.getComponent<CTransform>();
        CBoundingBox& box = rag.getComponent<CBoundingBox>();

        std::array<Vec2f, 4> vertices;
        float halfDiag = sqrtf(box.size.x * box.size.x + box.size.y * box.size.y) / 2.0f;
        float angleToVertex0 = asinf(box.halfSize.y / halfDiag); // bottom-right (without trans.angle)
        float angleToVertex1 = static_cast<float>(M_PI) - angleToVertex0; // bottom-left (without trans.angle)
        float angleToVertex2 = static_cast<float>(M_PI) + angleToVertex0;
        float angleToVertex3 = 2.0f * static_cast<float>(M_PI) - angleToVertex0;
        vertices[0] = trans.pos + Vec2f(cosf(angleToVertex0 + trans.angle) * halfDiag, sinf(angleToVertex0 + trans.angle) * halfDiag);
        vertices[1] = trans.pos + Vec2f(cosf(angleToVertex1 + trans.angle) * halfDiag, sinf(angleToVertex1 + trans.angle) * halfDiag);
        vertices[2] = trans.pos + Vec2f(cosf(angleToVertex2 + trans.angle) * halfDiag, sinf(angleToVertex2 + trans.angle) * halfDiag);
        vertices[3] = trans.pos + Vec2f(cosf(angleToVertex3 + trans.angle) * halfDiag, sinf(angleToVertex3 + trans.angle) * halfDiag);

        /// TODO: change entirely maybe, if too slow down the line
        std::array<Vec2f, 4> prevVertices;
        angleToVertex0 = asinf(box.halfSize.y / halfDiag);
        angleToVertex1 = static_cast<float>(M_PI) - angleToVertex0;
        angleToVertex2 = static_cast<float>(M_PI) + angleToVertex0;
        angleToVertex3 = 2.0f * static_cast<float>(M_PI) - angleToVertex0;
        prevVertices[0] = trans.prevPos + Vec2f(cosf(angleToVertex0 + trans.prevAngle) * halfDiag, sinf(angleToVertex0 + trans.prevAngle) * halfDiag);
        prevVertices[1] = trans.prevPos + Vec2f(cosf(angleToVertex1 + trans.prevAngle) * halfDiag, sinf(angleToVertex1 + trans.prevAngle) * halfDiag);
        prevVertices[2] = trans.prevPos + Vec2f(cosf(angleToVertex2 + trans.prevAngle) * halfDiag, sinf(angleToVertex2 + trans.prevAngle) * halfDiag);
        prevVertices[3] = trans.prevPos + Vec2f(cosf(angleToVertex3 + trans.prevAngle) * halfDiag, sinf(angleToVertex3 + trans.prevAngle) * halfDiag);

        // std::cout << "halfDiag: " << halfDiag << std::endl;
        // for (int i = 0; i < 4; ++i)
        // {
        //     std::cout << "angleToVertex: " << angleToVertex0 << " " << angleToVertex1 << " " << angleToVertex2 << " " << angleToVertex3 << std::endl;
        //     std::cout << "vertex " << i << ": " << vertices[i].x << " " << vertices[i].y << std::endl;
        // }

        for (size_t i = 0; i < 4; ++i)
        {
            Vec2f vert = vertices[i];
            Vec2f prevVert = prevVertices[i];
            Vec2uz gridPos = vert.to<size_t>() / static_cast<size_t>(m_cellSizePixels);

            // std::cout << "vertex " << i << ": " << vert << std::endl;

            /// TODO: edge case: vert x = 1300 so grid pos = 130, but no resolutions needs to happen
            /// TODO: maybe check multiple times per frame for more accuracy
            if (tiles[gridPos.x * m_worldMaxCells.y + gridPos.y].blocksMovement) // vertex inside tile /// TODO: possible seg faults
            {
                // std::cout << "collision with tile" << std::endl;

                float bounce = 0.6f;
                float friction = 0.4f; /// TODO: set based on tile type?
                float threshold = 0.00f;
                Vec2f travel = vert - prevVert;

                float xOverlap = m_cellSizePixels * 0.5f - abs(vert.x - (gridPos.x + 0.5f) * m_cellSizePixels);
                float yOverlap = m_cellSizePixels * 0.5f - abs(vert.y - (gridPos.y + 0.5f) * m_cellSizePixels);
                float xPrevOverlap = m_cellSizePixels * 0.5f - abs(prevVert.x - (gridPos.x + 0.5f) * m_cellSizePixels);
                float yPrevOverlap = m_cellSizePixels * 0.5f - abs(prevVert.y - (gridPos.y + 0.5f) * m_cellSizePixels);

                // if (xOverlap < yOverlap) // handle smallest overlap collisions first
                // {
                //     if (yPrevOverlap > 0) // collided from left or right
                //     {
                //         if (travel.x > 0) // collided from the left
                //         {
                //             trans.pos.x -= xOverlap * 0.5f;
                //             float normalForce = -travel.x * bounce;
                //             float frictionForce = -travel.y * friction;
                //             std::cout << "collided from left: " << normalForce << " " << frictionForce << "\n";
                //             if (abs(normalForce) >= threshold || abs(frictionForce) >= threshold)
                //                 Physics::ForceEntity(rag, Vec2f(normalForce, frictionForce), vert); /// TODO: add some sort of if force less than certain amount just make velocity zero so that is doesn't infinitely bounce at tiny bounce amounts
                //             // else
                //             // {
                //             //     std::cout << "skipping force" << std::endl;
                //             //     trans.velocity.x = 0;
                //             //     trans.velocity.y = 0;
                //             //     trans.angularVelocity = 0;
                //             // }
                //         }
                //         else if (travel.x < 0) // collided from the right
                //         {
                //             trans.pos.x += xOverlap * 0.5f;
                //             float normalForce = -travel.x * bounce;
                //             float frictionForce = -travel.y * friction;
                //             std::cout << "collided from right: " << normalForce << " " << frictionForce << "\n";
                //             if (abs(normalForce) >= threshold || abs(frictionForce) >= threshold)
                //                 Physics::ForceEntity(rag, Vec2f(normalForce, frictionForce), vert);
                //             // else
                //             // {
                //             //     std::cout << "skipping force" << std::endl;
                //             //     trans.velocity.x = 0;
                //             //     trans.velocity.y = 0;
                //             //     trans.angularVelocity = 0;
                //             // }
                //         }
                //         /// TODO: travel.x == 0?
                //     }
                //     else if (xPrevOverlap > 0) // collided from top or bottom
                //     {
                //         if (travel.y > 0) // collided from the top
                //         {
                //             trans.pos.y -= yOverlap * 0.5f; // dampening
                //             float normalForce = -travel.y * bounce;
                //             float frictionForce = -travel.x * friction;
                //             std::cout << "collided from top: " << normalForce << " " << frictionForce << "\n";
                //             if (abs(normalForce) >= threshold || abs(frictionForce) >= threshold)
                //                 Physics::ForceEntity(rag, Vec2f(frictionForce, normalForce), vert);
                //             // else
                //             // {
                //             //     std::cout << "skipping force" << std::endl;
                //             //     trans.velocity.x = 0;
                //             //     trans.velocity.y = 0;
                //             //     trans.angularVelocity = 0;
                //             // }
                //         }
                //         else if (travel.y < 0) // collided from the bottom
                //         {
                //             trans.pos.y += yOverlap * 0.5f;
                //             float normalForce = -travel.y * bounce;
                //             float frictionForce = -travel.x * friction;
                //             std::cout << "collided from bottom: " << normalForce << " " << frictionForce << "\n";
                //             if (abs(normalForce) >= threshold || abs(frictionForce) >= threshold)
                //                 Physics::ForceEntity(rag, Vec2f(frictionForce, normalForce), vert);
                //             // else
                //             // {
                //             //     std::cout << "skipping force" << std::endl;
                //             //     trans.velocity.x = 0;
                //             //     trans.velocity.y = 0;
                //             //     trans.angularVelocity = 0;
                //             // }
                //         }
                //         /// TODO: travel.y == 0?
                //     }
                // }
                // else
                // {
                if (yPrevOverlap <= 0) // collided from top or bottom
                {
                    if (travel.y > 0) // collided from the top
                    {
                        trans.pos.y -= yOverlap; /// TODO: could add dampening like * 0.5f

                        float normalForceMag = travel.y * bounce;
                        float frictionForceMag = std::min(normalForceMag * friction, abs(trans.velocity.x));
                        // std::cout << "collided from top: " << normalForceMag << " " << frictionForceMag << "\n";
                        if (abs(normalForceMag) >= threshold || abs(frictionForceMag) >= threshold)
                            Physics::ForceEntity(trans.pos, trans.velocity, trans.angularVelocity, box.size, Vec2f(travel.x > 0 ? -frictionForceMag : frictionForceMag, -normalForceMag), vert);
                        else
                        {
                            // std::cout << "skipping force" << std::endl;
                            // trans.velocity.x = 0;
                            // trans.velocity.y = 0;
                            // trans.angularVelocity = 0;
                        }
                    }
                    else if (travel.y < 0) // collided from the bottom
                    {
                        trans.pos.y += yOverlap;

                        float normalForceMag = -travel.y * bounce;
                        float frictionForceMag = std::min(normalForceMag * friction, abs(trans.velocity.x));
                        // std::cout << "collided from bottom: " << normalForceMag << " " << frictionForceMag << "\n";
                        if (abs(normalForceMag) >= threshold || abs(frictionForceMag) >= threshold)
                            Physics::ForceEntity(trans.pos, trans.velocity, trans.angularVelocity, box.size, Vec2f(travel.x > 0 ? -frictionForceMag : frictionForceMag, normalForceMag), vert);
                        // else
                        // {
                        //     std::cout << "skipping force" << std::endl;
                        //     trans.velocity.x = 0;
                        //     trans.velocity.y = 0;
                        //     trans.angularVelocity = 0;
                        // }
                    }
                    /// TODO: travel.y == 0?
                }
                else if (xPrevOverlap <= 0) // collided from left or right
                {
                    if (travel.x > 0) // collided from the left
                    {
                        trans.pos.x -= xOverlap;

                        float normalForceMag = travel.x * bounce;
                        float frictionForceMag = std::min(normalForceMag * friction, abs(trans.velocity.y));
                        // std::cout << "collided from left: " << normalForceMag << " " << frictionForceMag << "\n";
                        if (abs(normalForceMag) >= threshold || abs(frictionForceMag) >= threshold)
                            Physics::ForceEntity(trans.pos, trans.velocity, trans.angularVelocity, box.size, Vec2f(-normalForceMag, travel.y > 0 ? -frictionForceMag : frictionForceMag), vert); /// TODO: add some sort of if force less than certain amount just make velocity zero so that is doesn't infinitely bounce at tiny bounce amounts
                        // else
                        // {
                        //     std::cout << "skipping force" << std::endl;
                        //     trans.velocity.x = 0;
                        //     trans.velocity.y = 0;
                        //     trans.angularVelocity = 0;
                        // }
                    }
                    else if (travel.x < 0) // collided from the right
                    {
                        trans.pos.x += xOverlap;

                        float normalForceMag = -travel.x * bounce;
                        float frictionForceMag = std::min(normalForceMag * friction, abs(trans.velocity.y));
                        // std::cout << "collided from right: " << normalForceMag << " " << frictionForceMag << "\n";
                        if (abs(normalForceMag) >= threshold || abs(frictionForceMag) >= threshold)
                            Physics::ForceEntity(trans.pos, trans.velocity, trans.angularVelocity, box.size, Vec2f(normalForceMag, travel.y > 0 ? -frictionForceMag : frictionForceMag), vert);
                        // else
                        // {
                        //     std::cout << "skipping force" << std::endl;
                        //     trans.velocity.x = 0;
                        //     trans.velocity.y = 0;
                        //     trans.angularVelocity = 0;
                        // }
                    }
                    /// TODO: travel.x == 0?
                }
                else
                {
                    // std::cout << "no previous overlap, skipping\n";
                }
                // }
                /// TODO: no previous overlap?
                /// TODO: trans.pos close enough to trans.prevPos, then don't make a physics update, just freeze it until there is no collision again
            }
        }
    }
}

/// @brief handle all weapon firing logic (and melee if implemented) and projectile movement; decoupled from other entities since updated multiple times per frame; includes CInput, CFire, CTransform, CDamage, CHealth, CType, tile matrix
void ScenePlay::sProjectiles()
{
    PROFILE_FUNCTION();

    std::vector<Entity>& bullets = m_entityManager.getEntities("bullet");

    // move and possibly destroy existing projectiles first
    updateProjectiles(bullets);
    updateProjectiles(bullets);
    updateProjectiles(bullets);
    updateProjectiles(bullets);
    updateProjectiles(bullets);
    updateProjectiles(bullets);
    updateProjectiles(bullets);
    updateProjectiles(bullets);
    updateProjectiles(bullets);
    updateProjectiles(bullets);

    // then handle bullet spawning
    CInput& input = m_player.getComponent<CInput>();
    CFire& fire = m_weapon.getComponent<CFire>();
    std::chrono::steady_clock::time_point now = std::chrono::high_resolution_clock::now();
    if (input.shoot && (now - fire.lastShotTime).count() >= 1000000000 / fire.fireRate)
    {
        fire.lastShotTime = now;
        if (fire.accuracy > fire.minAccuracy)
        {
            fire.accuracy -= 0.003f;
        }
        spawnBullet(m_weapon);
    }
    else if (fire.accuracy < fire.maxAccuracy)
    {
        fire.accuracy += 0.0002f;
    }
}

/// TODO: grouping similar actions (e.g., input actions like "JUMP", "LEFT", "RIGHT", etc.) into an enum or constants to avoid potential typos and improve maintainability. This way, your if-else chains would be more scalable if new actions are added
/// @brief sets CInput variables according to action, no action logic here; includes CInput
void ScenePlay::sDoAction(const Action& action)
{
    PROFILE_FUNCTION();

    if (action.type() == START)
    {
        if (action.name() == "TOGGLE_TEXTURE")
        {
            m_drawTextures = !m_drawTextures;
        }
        else if (action.name() == "TOGGLE_COLLISION")
        {
            m_drawCollision = !m_drawCollision;
        }
        else if (action.name() == "TOGGLE_MAP")
        {
            m_drawMinimap = !m_drawMinimap;
        }
        else if (action.name() == "PAUSE")
        {
            setPaused();
        }
        else if (action.name() == "QUIT")
        {
            onEnd();
        }
        else if (action.name() == "JUMP")
        {
            m_player.getComponent<CInput>().up = true;
        }
        else if (action.name() == "LEFT")
        {
            m_player.getComponent<CInput>().left = true;
        }
        else if (action.name() == "RIGHT")
        {
            m_player.getComponent<CInput>().right = true;
        }
        else if (action.name() == "SHOOT")
        {
            m_player.getComponent<CInput>().shoot = true;
        }
    }
    else if (action.type() == END)
    {
        if (action.name() == "LEFT")
        {
            m_player.getComponent<CInput>().left = false;
        }
        else if (action.name() == "RIGHT")
        {
            m_player.getComponent<CInput>().right = false;
        }
        else if (action.name() == "JUMP")
        {
            m_player.getComponent<CInput>().up = false;
        }
        else if (action.name() == "SHOOT")
        {
            m_player.getComponent<CInput>().shoot = false;
        }
    }
}

/// @brief handles the behavior of NPCs
/// TODO: implement NPC AI follow and patrol behavior
void ScenePlay::sAI()
{
    PROFILE_FUNCTION();
}

/// TODO: finish this
/// @brief updates all entities' lifespan and whatever else status; includes CLifespan, CInvincibility
void ScenePlay::sStatus()
{
    PROFILE_FUNCTION();

    /// TODO: do same locational thing here as with collision and tileMatrix[x][y]
    /// TODO: may want to separate lifespan and health since shit is stored so that components are cached together, or change the way components and entities are stored

    // bullets lifespan
    for (Entity& e : m_entityManager.getEntities("bullet"))
    {
        int& lifespan = e.getComponent<CLifespan>().lifespan;
        if (lifespan <= 0)
        {
            e.destroy();
        }
        else
        {
            --lifespan;
        }
    }

    // players have invincibility times
    for (Entity& e : m_entityManager.getEntities("player"))
    {
        int& invTime = e.getComponent<CInvincibility>().timeRemaining;
        if (invTime > 0)
        {
            --invTime;
        }
    }

    /// TODO: old code that may still be viable, test against current code later, good to iterate through single components at a time for memory speed
    // for (Entity& e : m_entityManager.getEntities())
    // {
    //     // lifespan
    //     if (e.hasComponent<CLifespan>())
    //     {
    //         int& lifespan = e.getComponent<CLifespan>().lifespan;
    //         if (lifespan <= 0)
    //         {
    //             e.destroy();
    //         }
    //         else
    //         {
    //             lifespan--;
    //         }
    //     }

    //     // health
    //     if (e.hasComponent<CHealth>())
    //     {
    //         int& health = e.getComponent<CHealth>().current;
    //         if (health <= 0)
    //         {
    //             e.destroy();
    //         }
    //     }
    // }
}

/// @brief handles all entities' animation updates
void ScenePlay::sAnimation()
{
    PROFILE_FUNCTION();
    /// TODO: could create an "animation" which is just data on where each limb should be and what angle it should be at, and could then use this data to control rigid bodies when alive by forcing them toward the angle and position needed. Easy transition to ragdoll from there by just letting the entity bodies fall (they're already created, in the right places, and have the right velocities to have a smooth transition)
    CAnimation& playerAnim = m_player.getComponent<CAnimation>();
    CState& playerState = m_player.getComponent<CState>();
    if (playerState.state == State::RUN)
    {
        playerAnim.animation.updateLoop();
    }


    /// TODO: if animation is not repeated, and it has ended, destroy the entity

    // set animation of player based on its CState component
}

/// @brief handles camera view logic; includes CTransform
void ScenePlay::sCamera()
{
    PROFILE_FUNCTION();

    Vec2f& pPos = m_player.getComponent<CTransform>().pos;

    // center the view on the player
    const Vec2i& viewSize { static_cast<int>(m_game.window().getSize().x), static_cast<int>(m_game.window().getSize().y) };
    float viewCenterX = std::clamp(pPos.x, viewSize.x / 2.0f, m_worldMaxPixels.x - viewSize.x / 2.0f);
    float viewCenterY = std::clamp(pPos.y, viewSize.y / 2.0f, m_worldMaxPixels.y - viewSize.y / 2.0f);
    m_mainView.setCenter({ viewCenterX, viewCenterY });
    m_miniMapView.setCenter({ viewCenterX / m_cellSizePixels, viewCenterY / m_cellSizePixels });

    // move the camera slightly toward the players mouse position (capped at a max displacement)
    // const Vec2i& mousePosOnWindow = sf::Mouse::getPosition(m_game.window());
    // float dx = std::clamp((mousePosOnWindow.x - viewSize.x / 2.0f) * 0.15f, -25.0f, 25.0f);
    // float dy = std::clamp((mousePosOnWindow.y - viewSize.y / 2.0f) * 0.15f, -25.0f, 25.0f);
    // view.move({ dx, dy });
}

/// @brief handles all rendering of textures (animations), grid boxes, collision boxes, and fps counter; includes CTransform, CAnimation, tile matrix
void ScenePlay::sRender()
{
    PROFILE_FUNCTION();

    sf::RenderWindow& window = m_game.window();
    window.setView(m_mainView);
    window.clear(sf::Color(10, 10, 10));

    const CTransform& playerTrans = m_player.getComponent<CTransform>();
    std::vector<Tile>& tiles = m_tileManager.getTiles();

    // collidable layer (tiles, player, bullets, items), this comes last so it's always visible
    Vec2i playerGridPos = (playerTrans.pos / m_cellSizePixels).to<int>(); // signed, for operations below /// NOTE: grid pos 0 means pixel 0 through 9

    const Vec2f mainViewSize { m_mainView.getSize().x, m_mainView.getSize().y }; //  window size is the view size now

    Vec2i checkLength { (mainViewSize / m_cellSizePixels / 2.0f).to<int>() }; // half the view size in grid coords /// TODO: could use a different method to get the size of the view, this is just a quick fix for now

    // limits on grid coords to check, arguments to std::min and max must be signed for proper logic
    size_t minX = static_cast<size_t>(std::max(0, playerGridPos.x - checkLength.x));
    size_t maxX = static_cast<size_t>(std::min(static_cast<int>(m_worldMaxCells.x) - 1, playerGridPos.x + checkLength.x));
    size_t minY = static_cast<size_t>(std::max(0, playerGridPos.y - checkLength.y));
    size_t maxY = static_cast<size_t>(std::min(static_cast<int>(m_worldMaxCells.y) - 1, playerGridPos.y + checkLength.y));

    // find open air tiles method for visible tiles
    std::vector<Vec2i> openTiles; /// TODO: use these for vertices method, might not even need it and could just use visited
    std::stack<Vec2i> tileStack;
    /// @todo use of vector<bool> for visited tiles is not the best, could use a bitset or something else
    /// @todo could also consider not initializing it but just reserving it to the size needed
    std::vector<std::vector<bool>> visited { static_cast<size_t>(mainViewSize.x / m_cellSizePixels + 1),
                                             std::vector<bool>(static_cast<size_t>(mainViewSize.y / m_cellSizePixels + 1)) }; // + 1 because including min and max values /// TODO: could add a visited property to each tile instead but this is good for now

    /// TODO: could use rough ray tracing (fixed number of rays in all directions) to get the set of visible tiles (not all open like this) and then use those tiles with good ray tracing, then still render everything or do light prop or whatever
    /// TODO: could even keep this and render only tiles with ray trace vertices
    {
        PROFILE_SCOPE("find open tiles");
        findOpenTiles(static_cast<size_t>(playerGridPos.x), static_cast<size_t>(playerGridPos.y), minX, maxX, minY, maxY, tiles, openTiles, tileStack, visited);
    }

    /// TODO: slow, could use some other sort of logic (either just logic same process or different entirely like lighting based on distance to player and/or light cone direction) after taking another look at the recrsive func efficiency
    /// TODO: test rendering tiles underneath that are only along the line of sight of player, not just using the playerGridPos.x >= currentCoord.x then render below thing
    {
        PROFILE_SCOPE("blocks");

        /// TODO: test speed with texture and sprite, drawing to a texture pixel by pixel (taking advantage of single-pixel tiles), scaling, then rendering
        sf::VertexArray blocks(sf::PrimitiveType::Triangles);
        sf::Color c;

        // method of rendering only tiles that have a vertex in triangle fan
        // for (const Vec2f& vert : triangleFan)
        // {
        //     Vec2i gridCoord = vert.to<int>() / m_cellSizePixels;
        //     if (!(gridCoord.x <= minX || gridCoord.y <= minY || gridCoord.x >= maxX || gridCoord.y >= maxY))
        //     {
        //         if (tileMatrix[gridCoord.x][gridCoord.y].isActive())
        //         {
        //             // CColor& color = tileMatrix[gridCoord.x][gridCoord.y].getComponent<CColor>();
        //             // color.light = 255;
        //             // c.r = color.r;
        //             // c.g = color.g;
        //             // c.b = color.b;
        //             // c.a = color.light;
        //             // int px = gridCoord.x * m_cellSizePixels;
        //             // int py = gridCoord.y * m_cellSizePixels;
        //             // blocks.append({ Vec2f(px, py), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py), c });
        //             // blocks.append({ Vec2f(px, py + m_cellSizePixels), c });
        //             // blocks.append({ Vec2f(px, py + m_cellSizePixels), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py + m_cellSizePixels), c });
        //             propagateLight(blocks, 3, 0, gridCoord, gridCoord, minX, maxX, minY, maxY);
        //         }
        //         if (tileMatrix[gridCoord.x - 1][gridCoord.y].isActive())
        //         {
        //             // CColor& color = tileMatrix[gridCoord.x - 1][gridCoord.y].getComponent<CColor>();
        //             // color.light = 255;
        //             // c.r = color.r;
        //             // c.g = color.g;
        //             // c.b = color.b;
        //             // c.a = color.light;
        //             // int px = (gridCoord.x - 1) * m_cellSizePixels;
        //             // int py = gridCoord.y * m_cellSizePixels;
        //             // blocks.append({ Vec2f(px, py), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py), c });
        //             // blocks.append({ Vec2f(px, py + m_cellSizePixels), c });
        //             // blocks.append({ Vec2f(px, py + m_cellSizePixels), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py + m_cellSizePixels), c });
        //             propagateLight(blocks, 3, 0, Vec2i(gridCoord.x - 1, gridCoord.y), Vec2i(gridCoord.x - 1, gridCoord.y), minX, maxX, minY, maxY);
        //         }
        //         if (tileMatrix[gridCoord.x][gridCoord.y - 1].isActive())
        //         {
        //             // CColor& color = tileMatrix[gridCoord.x][gridCoord.y - 1].getComponent<CColor>();
        //             // color.light = 255;
        //             // c.r = color.r;
        //             // c.g = color.g;
        //             // c.b = color.b;
        //             // c.a = color.light;
        //             // int px = gridCoord.x * m_cellSizePixels;
        //             // int py = (gridCoord.y - 1) * m_cellSizePixels;
        //             // blocks.append({ Vec2f(px, py), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py), c });
        //             // blocks.append({ Vec2f(px, py + m_cellSizePixels), c });
        //             // blocks.append({ Vec2f(px, py + m_cellSizePixels), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py + m_cellSizePixels), c });
        //             propagateLight(blocks, 3, 0, Vec2i(gridCoord.x, gridCoord.y - 1), Vec2i(gridCoord.x, gridCoord.y - 1), minX, maxX, minY, maxY);
        //         }
        //         if (tileMatrix[gridCoord.x - 1][gridCoord.y - 1].isActive())
        //         {
        //             // CColor& color = tileMatrix[gridCoord.x - 1][gridCoord.y - 1].getComponent<CColor>();
        //             // color.light = 255;
        //             // c.r = color.r;
        //             // c.g = color.g;
        //             // c.b = color.b;
        //             // c.a = color.light;
        //             // int px = (gridCoord.x - 1) * m_cellSizePixels;
        //             // int py = (gridCoord.y - 1) * m_cellSizePixels;
        //             // blocks.append({ Vec2f(px, py), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py), c });
        //             // blocks.append({ Vec2f(px, py + m_cellSizePixels), c });
        //             // blocks.append({ Vec2f(px, py + m_cellSizePixels), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py), c });
        //             // blocks.append({ Vec2f(px + m_cellSizePixels, py + m_cellSizePixels), c });
        //             propagateLight(blocks, 3, 0, Vec2i(gridCoord.x - 1, gridCoord.y - 1), Vec2i(gridCoord.x - 1, gridCoord.y - 1), minX, maxX, minY, maxY);
        //         }
        //     }
        // }

        // method of rendering all visited tiles
        /// TODO: render tiles with no tile above them and 1 or two tiles missing to the side as ramps and deal with that accordingly in collisions
        for (size_t x = minX; x <= maxX; ++x)
        {
            for (size_t y = minY; y <= maxY; ++y)
            {
                if (visited[x - minX][y - minY])
                {
                    Tile& tile = tiles[x * m_worldMaxCells.y + y];
                    if (tile.health)
                    {
                        // Vec2i startCoord(x, y);
                        // Vec2i currentCoord = startCoord;
                        // propagateLight(blocks, 3, 0, startCoord, currentCoord, minX, maxX, minY, maxY);

                        tile.light = 255;
                        c.r = tile.r;
                        c.g = tile.g;
                        c.b = tile.b;
                        c.a = tile.light;
                        addBlock(blocks, static_cast<int>(x), static_cast<int>(y), c);

                        // draw neighbors with less lighting
                        // if (x > minX) // && playerGridPos.x >= x)
                        // {
                        //     Tile& neighborTile = tiles[(x - 1) * m_worldMaxCells.y + y];
                        //     if (neighborTile.health)
                        //     {
                        //         if (neighborTile.light < tile.light)
                        //         {
                        //             neighborTile.light = tile.light - 85;
                        //             c.a = neighborTile.light;
                        //             addBlock(blocks, x - 1, y, c);

                        //             if (x - 1 > minX)
                        //             {
                        //                 Tile& nextNeighborTile = tiles[(x - 2) * m_worldMaxCells.y + y];
                        //                 if (nextNeighborTile.health)
                        //                 {
                        //                     if (nextNeighborTile.light < neighborTile.light)
                        //                     {
                        //                         nextNeighborTile.light = neighborTile.light - 85;
                        //                         c.a = nextNeighborTile.light;
                        //                         addBlock(blocks, x - 2, y, c);
                        //                     }
                        //                 }

                        //                 // if (y > minY) // && playerGridPos.y >= y)
                        //                 // {
                        //                 //     const Entity& tile = tileMatrix[x - 2][y - 1];
                        //                 //     if (tile.health)
                        //                 //     {
                        //                 //         CColor& nextNeighborColor = tile.getComponent<CColor>();
                        //                 //         if (nextNeighborColor.light < neighborColor.light)
                        //                 //         {
                        //                 //             nextNeighborColor.light = neighborColor.light - 60;
                        //                 //             block.setFillColor(sf::Color(nextNeighborColor.r, nextNeighborColor.g, nextNeighborColor.b, nextNeighborColor.light));
                        //                 //             block.setPosition({ static_cast<float>(x - 2 - minX), static_cast<float>(y - 1 - minY) });
                        //                 //             m_tileTexture.draw(block);
                        //                 //         }
                        //                 //     }
                        //                 // }

                        //                 // if (y < maxY) // && playerGridPos.y <= y)
                        //                 // {
                        //                 //     const Entity& tile = tileMatrix[x - 2][y + 1];
                        //                 //     if (tile.health)
                        //                 //     {
                        //                 //         CColor& nextNeighborColor = tile.getComponent<CColor>();
                        //                 //         if (nextNeighborColor.light < neighborColor.light)
                        //                 //         {
                        //                 //             nextNeighborColor.light = neighborColor.light - 60;
                        //                 //             block.setFillColor(sf::Color(nextNeighborColor.r, nextNeighborColor.g, nextNeighborColor.b, nextNeighborColor.light));
                        //                 //             block.setPosition({ static_cast<float>(x - 2 - minX), static_cast<float>(y + 1 - minY) });
                        //                 //             m_tileTexture.draw(block);
                        //                 //         }
                        //                 //     }
                        //                 // }
                        //             }

                        //             // if (y > minY) // && playerGridPos.y >= y)
                        //             // {
                        //             //     const Entity& tile = tileMatrix[x - 1][y - 1];
                        //             //     if (tile.health)
                        //             //     {
                        //             //         CColor& nextNeighborColor = tile.getComponent<CColor>();
                        //             //         if (nextNeighborColor.light < neighborColor.light)
                        //             //         {
                        //             //             nextNeighborColor.light = neighborColor.light - 85;
                        //             //             c.a = nextNeighborColor.light;
                        //             //             blocks.append({ Vec2f(px - m_cellSizePixels, py - m_cellSizePixels), c });
                        //             //             blocks.append({ Vec2f(px, py - m_cellSizePixels), c });
                        //             //             blocks.append({ Vec2f(px - m_cellSizePixels, py), c });
                        //             //             blocks.append({ Vec2f(px - m_cellSizePixels, py), c });
                        //             //             blocks.append({ Vec2f(px, py - m_cellSizePixels), c });
                        //             //             blocks.append({ Vec2f(px, py), c });
                        //             //         }
                        //             //     }
                        //             // }

                        //             // if (y < maxY) // && playerGridPos.y <= y)
                        //             // {
                        //             //     const Entity& tile = tileMatrix[x - 1][y + 1];
                        //             //     if (tile.health)
                        //             //     {
                        //             //         CColor& nextNeighborColor = tile.getComponent<CColor>();
                        //             //         if (nextNeighborColor.light < neighborColor.light)
                        //             //         {
                        //             //             nextNeighborColor.light = neighborColor.light - 85;
                        //             //             c.a = nextNeighborColor.light;
                        //             //             blocks.append({ Vec2f(px - m_cellSizePixels, py + m_cellSizePixels), c });
                        //             //             blocks.append({ Vec2f(px, py + m_cellSizePixels), c });
                        //             //             blocks.append({ Vec2f(px - m_cellSizePixels, py + 2 * m_cellSizePixels), c });
                        //             //             blocks.append({ Vec2f(px - m_cellSizePixels, py + 2 * m_cellSizePixels), c });
                        //             //             blocks.append({ Vec2f(px, py + m_cellSizePixels), c });
                        //             //             blocks.append({ Vec2f(px, py + 2 * m_cellSizePixels), c });
                        //             //         }
                        //             //     }
                        //             // }
                        //         }
                        //     }
                        // }

                        // if (x < maxX) // && playerGridPos.x <= x)
                        // {
                        //     Tile& neighborTile = tiles[(x + 1) * m_worldMaxCells.y + y];
                        //     if (neighborTile.health)
                        //     {
                        //         if (neighborTile.light < tile.light)
                        //         {
                        //             neighborTile.light = tile.light - 85;
                        //             c.a = neighborTile.light;
                        //             addBlock(blocks, x + 1, y, c);

                        //             if (x + 1 < maxX)
                        //             {
                        //                 Tile& nextNeighborTile = tiles[(x + 2) * m_worldMaxCells.y + y];
                        //                 if (nextNeighborTile.health)
                        //                 {
                        //                     if (nextNeighborTile.light < neighborTile.light)
                        //                     {
                        //                         nextNeighborTile.light = neighborTile.light - 85;
                        //                         c.a = nextNeighborTile.light;
                        //                         addBlock(blocks, x + 2, y, c);
                        //                     }
                        //                 }
                        //             }
                        //         }
                        //     }
                        // }

                        // if (y > minY) // && playerGridPos.y >= y)
                        // {
                        //     Tile& neighborTile = tiles[x * m_worldMaxCells.y + (y - 1)];
                        //     if (neighborTile.health)
                        //     {
                        //         if (neighborTile.light < tile.light)
                        //         {
                        //             neighborTile.light = tile.light - 85;
                        //             c.a = neighborTile.light;
                        //             addBlock(blocks, x, y - 1, c);

                        //             if (y - 1 > minY)
                        //             {
                        //                 Tile& nextNeighborTile = tiles[x + m_worldMaxCells.y + (y - 2)];
                        //                 if (nextNeighborTile.health)
                        //                 {
                        //                     if (nextNeighborTile.light < neighborTile.light)
                        //                     {
                        //                         nextNeighborTile.light = neighborTile.light - 85;
                        //                         c.a = nextNeighborTile.light;
                        //                         addBlock(blocks, x, y - 2, c);
                        //                     }
                        //                 }
                        //             }
                        //         }
                        //     }
                        // }

                        // if (y < maxY) // && playerGridPos.y <= y)
                        // {
                        //     Tile& neighborTile = tiles[x * m_worldMaxCells.y + (y + 1)];
                        //     if (neighborTile.health)
                        //     {
                        //         if (neighborTile.light < tile.light)
                        //         {
                        //             neighborTile.light = tile.light - 85;
                        //             c.a = neighborTile.light;
                        //             addBlock(blocks, x, y + 1, c);

                        //             if (y + 1 < maxY)
                        //             {
                        //                 Tile& nextNeighborTile = tiles[x * m_worldMaxCells.y + (y + 2)];
                        //                 if (nextNeighborTile.health)
                        //                 {
                        //                     if (nextNeighborTile.light < neighborTile.light)
                        //                     {
                        //                         nextNeighborTile.light = neighborTile.light - 85;
                        //                         c.a = nextNeighborTile.light;
                        //                         addBlock(blocks, x, y + 2, c);
                        //                     }
                        //                 }
                        //             }
                        //         }
                        //     }
                        // }
                    }
                    /// TODO: render visited backgrounds and decoration and such differently if they arent 10x10 sized pixels only (static background blocks can be I suppose, but not the main parallax background)
                    // else render other shit that's not tiles (background, etc)
                }
            }
        }

        window.draw(blocks);

        // reset lighting for light propagation /// TODO: consider using a visited array, maybe the same as used for findOpenTiles instead of using this light thing (adds this loop)
        // for (int x = minX; x <= maxX; ++x)
        // {
        //     for (int y = minY; y <= maxY; ++y)
        //     {
        //         tiles[x * m_worldMaxCells.y + y].light = 0;
        //     }
        // }
    }

    // ray casting
    std::vector<Vec2f> triangleFan = rayCast(Vec2f(m_mainView.getCenter().x, m_mainView.getCenter().y), Vec2f(mainViewSize.x, mainViewSize.y), openTiles, playerTrans.pos, tiles, minX, maxX, minY, maxY);
    sf::VertexArray fan(sf::PrimitiveType::TriangleFan, triangleFan.size());
    for (size_t i = 0; i < triangleFan.size(); ++i)
    {
        fan[i].position = sf::Vector2f { triangleFan[i].x, triangleFan[i].y };
        fan[i].color = sf::Color { 255, 255, 255, 50 };

        // sf::CircleShape dot(2);
        // dot.setPosition({ static_cast<float>(triangleFan[i].x - 2), static_cast<float>(triangleFan[i].y - 2) });
        // dot.setFillColor(sf::Color(0, 0, 255, 100));
        // window.draw(dot);
    }
    window.draw(fan);

    // bullets
    for (Entity& bullet : m_entityManager.getEntities("bullet"))
    {
        const CTransform& transform = bullet.getComponent<CTransform>();

        sf::Sprite& sprite = bullet.getComponent<CAnimation>().animation.getSprite();
        sprite.setRotation(sf::radians(transform.angle));
        sprite.setPosition({ transform.pos.x, transform.pos.y });
        sprite.setScale({ transform.scale.x, transform.scale.y });

        window.draw(sprite);
    }

    // ragdolls
    for (Entity& rag : m_entityManager.getEntities("ragdoll"))
    {
        const CTransform& trans = rag.getComponent<CTransform>();
        sf::Sprite& sprite = rag.getComponent<CAnimation>().animation.getSprite();
        sprite.setPosition({ trans.pos.x, trans.pos.y });
        sprite.setRotation(sf::radians(trans.angle));
        window.draw(sprite);

        // draw bounding box
        const CBoundingBox& box = rag.getComponent<CBoundingBox>();
        sf::RectangleShape rect;
        rect.setSize({ box.size.x - 1.0f, box.size.y - 1.0f }); // - 1 cuz line thickness of 1?
        rect.setOrigin({ box.halfSize.x, box.halfSize.y });
        rect.setPosition({ trans.pos.x, trans.pos.y });
        rect.setRotation(sf::radians(trans.angle));
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineColor(sf::Color::White);
        rect.setOutlineThickness(1);
        window.draw(rect);
    }

    // player, player parts, and weapon held
    if (m_player.isActive())
    {
        // back arm
        const CTransform& backArmTrans = m_playerArmBack.getComponent<CTransform>();
        sf::Sprite& backArmSprite = m_playerArmBack.getComponent<CAnimation>().animation.getSprite();
        backArmSprite.setPosition({ backArmTrans.pos.x, backArmTrans.pos.y });
        backArmSprite.setScale({ backArmTrans.scale.x, backArmTrans.scale.y });
        backArmSprite.setRotation(sf::radians(backArmTrans.angle));
        window.draw(backArmSprite);

        // player
        sf::Sprite& playerSprite = m_player.getComponent<CAnimation>().animation.getSprite();
        playerSprite.setPosition({ playerTrans.pos.x, playerTrans.pos.y });
        playerSprite.setScale({ playerTrans.scale.x, playerTrans.scale.y });
        window.draw(playerSprite);

        // health bar
        sf::RectangleShape healthBarOutline({ 30, 5 });
        CBoundingBox& playerBox = m_player.getComponent<CBoundingBox>();
        healthBarOutline.setPosition({ playerTrans.pos.x - 15, playerTrans.pos.y - playerBox.halfSize.y - 15 });
        healthBarOutline.setOutlineColor(sf::Color::White);
        healthBarOutline.setOutlineThickness(1);
        healthBarOutline.setFillColor(sf::Color::Transparent);
        CHealth& playerHealth = m_player.getComponent<CHealth>();
        sf::RectangleShape healthBar({ static_cast<float>(playerHealth.current) / static_cast<float>(playerHealth.max) * 30, 5 });
        healthBar.setPosition({ playerTrans.pos.x - 15, playerTrans.pos.y - playerBox.halfSize.y - 15 });
        healthBar.setFillColor(sf::Color::Red);
        window.draw(healthBarOutline);
        window.draw(healthBar);

        // weapon
        const CTransform& weaponTrans = m_weapon.getComponent<CTransform>();
        sf::Sprite& weaponSprite = m_weapon.getComponent<CAnimation>().animation.getSprite();
        weaponSprite.setPosition({ weaponTrans.pos.x, weaponTrans.pos.y });
        weaponSprite.setScale({ weaponTrans.scale.x, weaponTrans.scale.y });
        weaponSprite.setRotation(sf::radians(weaponTrans.angle));
        window.draw(weaponSprite);

        // head
        const CTransform& headTrans = m_playerHead.getComponent<CTransform>();
        sf::Sprite& headSprite = m_playerHead.getComponent<CAnimation>().animation.getSprite();
        headSprite.setPosition({ headTrans.pos.x, headTrans.pos.y });
        headSprite.setScale({ headTrans.scale.x, headTrans.scale.y });
        headSprite.setRotation(sf::radians(headTrans.angle));
        window.draw(headSprite);

        // front arm
        const CTransform& frontArmTrans = m_playerArmFront.getComponent<CTransform>();
        sf::Sprite& frontArmSprite = m_playerArmFront.getComponent<CAnimation>().animation.getSprite();
        frontArmSprite.setPosition({ frontArmTrans.pos.x, frontArmTrans.pos.y });
        frontArmSprite.setScale({ frontArmTrans.scale.x, frontArmTrans.scale.y });
        frontArmSprite.setRotation(sf::radians(frontArmTrans.angle));
        window.draw(frontArmSprite);
    }

    /// normal rendering without vision lighting:
    // for (int x = minX; x <= maxX; ++x)
    // {
    //     for (int y = minY; y <= maxY; ++y)
    //     {
    //         const Entity& tile = tileMatrix[x][y];
    //         if (tile.isActive())
    //         {
    //             sf::RectangleShape block = sf::RectangleShape(GlobalSettings::cellSizePixels.to<float>());
    //             CColor& color = tile.getComponent<CColor>();
    //             block.setFillColor(sf::Color(color.r, color.g, color.b));
    //             block.setPosition({ static_cast<float>(x * m_cellSizePixels.x), static_cast<float>(y * m_cellSizePixels.y) });
    //             window.draw(block);
    //             // CTransform& trans = tile.getComponent<CTransform>();
    //             // sf::Sprite& sprite = tile.getComponent<CAnimation>().animation.getSprite();
    //             // sprite.setRotation(sf::radians(trans.rotAngle)); /// TODO: may not even need this either, but may want it for better physics
    //             // sprite.setPosition(trans.pos);
    //             // sprite.setScale(trans.scale); /// TODO: will I even use scale?
    //             // window.draw(sprite);
    //         }
    //     }
    // }

    /// fps counter
    float elapsedTime = m_fpsClock.restart().asSeconds();
    float fps = 1.0f / elapsedTime;
    m_fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));
    window.setView(window.getDefaultView());
    window.draw(m_fpsText);

    /// crosshair /// TODO:

    /// TODO: line of sight without cone? triangle fan method, ray casting
    // ideas:
        // 1. player's current (x, y), then do some sort of search or something to find all end nodes where tile is active and check only those
        // 1.1. cast rays to vertices of those tiles only (can manage) and do triangle fan method

        // 3. turn tile map into polygon with only outside edges (has other physics advantages as well for collisions and such and can have edges that don't align with axes and so on), only regen poly map if tile map changes, then use vertices from poly map with 3 ray per vertex method or my angle offset method implemented now, could do line segment intersections (implemented in Physics) or incremental tile checks like implemented now
        // 3.1. https://www.youtube.com/watch?v=fc3nnG2CG8U&t=17s&ab_channel=javidx9

    /// TODO: light cone: see in direction of pointer only
    // get mouse pointer pos
    // create 2 vectors at angles -x and x away from player-to-pointer vector
    // line segment from player to edge of screen and check for intersection, first intersection visible, rest not
    // could elim the findOpenTiles and just shine enough rays out in dir of pointer to hit all visible tiles, then use those with limited angle triangle fan
    // use a shader?
    // triangle fan with limited angle
    // entity can see up to closest intersection point
    // lighting effects : light travels outward in all directions
    // cast rays at all vertices, shine two extras for each ray just to the left and right of it to have rays that go all the way to the edges of the screen, then connect all endpoints of lines and fill in the light
    // can shine a few more rays to get a shadow look
    // may be able to do this my own way since places where light should extend(but wouldn't with the connect-the-dots method if not using the extra two ways for every vertex-aiming ray) do not have a ray that intersects a line between them

    /// minimap
    /// TODO: use single small texture for this and scale it or use vertexarray with points (if they're pixels) that are scaled or triangles
    /// TODO: consider using sf::Image, look into it; if not, create small and render scaled, just like tiles in main game
    if (m_drawMinimap)
    {
        PROFILE_SCOPE("rendering minimap");

        window.setView(m_miniMapView);
        const sf::Vector2f& miniViewSize = m_miniMapView.getSize();

        // background
        sf::RectangleShape minimapBackground(miniViewSize);
        minimapBackground.setPosition({ m_miniMapView.getCenter().x - miniViewSize.x / 2.0f, m_miniMapView.getCenter().y - miniViewSize.y / 2.0f });
        minimapBackground.setFillColor(sf::Color(50, 50, 50));
        window.draw(minimapBackground);

        // player icon
        sf::CircleShape player(5);
        player.setFillColor(sf::Color::Green);
        player.setPosition({ m_miniMapView.getCenter().x - 5, m_miniMapView.getCenter().y - 5 });
        window.draw(player);

        /// @todo change this later to use some set zoom or something for the minimap (have a separate view for opening the whole map)
        minX = static_cast<size_t>(std::max(0, playerGridPos.x - checkLength.x * 10));
        maxX = static_cast<size_t>(std::min(static_cast<int>(m_worldMaxCells.x) - 1, playerGridPos.x + checkLength.x * 10));
        minY = static_cast<size_t>(std::max(0, playerGridPos.y - checkLength.y * 10));
        maxY = static_cast<size_t>(std::min(static_cast<int>(m_worldMaxCells.y) - 1, playerGridPos.y + checkLength.y * 10));

        sf::VertexArray points(sf::PrimitiveType::Points);
        sf::Color c;
        for (size_t x = minX; x <= maxX * 10; ++x)
        {
            for (size_t y = minY; y <= maxY; ++y)
            {
                Tile& tile = tiles[x * m_worldMaxCells.y + y];

                if (tile.blocksMovement || tile.blocksVision)
                {
                    c.r = tile.r;
                    c.g = tile.g;
                    c.b = tile.b;
                    sf::Vertex v = { sf::Vector2f(x, y), c };
                    points.append(v);
                }
            }
        }
        window.draw(points);
    }

    window.display();
    window.setView(m_mainView);
}

/**
 * helper functions
 */

 /// @brief returns the midpoint of entity based on a given grid position
 /// TODO: eliminating this and just using the top-left positioning like SFML would probably save me seom computation time
Vec2f ScenePlay::gridToMidPixel(float gridX, float gridY, Entity entity)
{
    PROFILE_FUNCTION();

    const Vec2f& bBoxHalfSize = entity.getComponent<CBoundingBox>().halfSize;

    float xPos = gridX * m_cellSizePixels + bBoxHalfSize.x;
    float yPos = gridY * m_cellSizePixels + bBoxHalfSize.y;

    return Vec2f(xPos, yPos);
}

// return Vec2f indicating where the pos (top-left corner) of the entity should be, (0, 0) is top-left corner of window
// Vec2f ScenePlay::gridToPixel(float gridX, float gridY)
// {
//     float xPos = gridX * m_cellSizePixels.x;
//     float yPos = gridY * m_cellSizePixels.y;
// }

/// @brief spawns the player entity
void ScenePlay::spawnPlayer()
{
    PROFILE_FUNCTION();

    // set player components
    m_player = m_entityManager.addEntity("player");
    m_player.addComponent<CBoundingBox>(Vec2i { m_playerConfig.CW, m_playerConfig.CH }, true, true);
    CTransform& playerTrans = m_player.addComponent<CTransform>(gridToMidPixel(m_worldMaxCells.x / 2, m_worldMaxCells.y / 2, m_player)); // must be after bounding box /// TODO: make spawning in dynamic
    playerTrans.scale = Vec2f(0.5f, 0.5f);
    m_player.addComponent<CAnimation>(m_game.assets().getAnimation("Run"), false);
    m_player.addComponent<CState>(State::AIR);
    m_player.addComponent<CInput>();
    m_player.addComponent<CGravity>(m_playerConfig.GRAVITY);
    m_player.addComponent<CInvincibility>(30); // in frames for now, will change /// TODO: that
    m_player.addComponent<CHealth>(100);

    // send spawn to network
    // NetworkData data {
    //     .dataType = NetworkData::DataType::SPAWN,
    //     .first.id = m_player.getID(),
    //     .second.f = playerTrans.pos.x,
    //     .third.f = playerTrans.pos.y
    // };
    // m_game.getNetManager().sendData(data);

    // spawn player arms
    m_playerArmBack = m_entityManager.addEntity("playerPart");
    CTransform& armBackTrans = m_playerArmBack.addComponent<CTransform>(); // must be after bounding box /// TODO: make spawning in dynamic
    armBackTrans.scale = Vec2f(0.5f, 0.5f);
    m_playerArmBack.addComponent<CAnimation>(m_game.assets().getAnimation("ArmBack"), false);

    m_playerArmFront = m_entityManager.addEntity("playerPart");
    CTransform& armFrontTrans = m_playerArmFront.addComponent<CTransform>(); // must be after bounding box /// TODO: make spawning in dynamic
    armFrontTrans.scale = Vec2f(0.5f, 0.5f);
    m_playerArmFront.addComponent<CAnimation>(m_game.assets().getAnimation("ArmFront"), false);

    // player head
    m_playerHead = m_entityManager.addEntity("playerPart");
    CTransform& headTrans = m_playerHead.addComponent<CTransform>(); // must be after bounding box /// TODO: make spawning in dynamic
    headTrans.scale = Vec2f(0.4f, 0.4f);
    m_playerHead.addComponent<CAnimation>(m_game.assets().getAnimation("Head"), false);

    // spawn player weapon
    m_weapon = m_entityManager.addEntity("weapon");
    m_weapon.addComponent<CFire>(50, 0.97f, 1.0f);
    m_weapon.addComponent<CDamage>(50);
    m_weapon.addComponent<CTransform>(m_player.getComponent<CTransform>().pos).scale = Vec2f(0.3f, 0.3f); /// TODO: make this a lil infront of player
    m_weapon.addComponent<CBoundingBox>(Vec2i(50, 10), false, false); /// TODO: make this dynamic for each weapon
    m_weapon.addComponent<CAnimation>(m_game.assets().getAnimation("Weapon"), false);
    /// TODO: add animation, gravity, bounding box, transform, state, etc. since weapons will drop from player on death
}

/// @brief spawn a bullet at the location of entity traveling toward cursor
void ScenePlay::spawnBullet(Entity entity)
{
    PROFILE_FUNCTION();

    CTransform& entityTrans = entity.getComponent<CTransform>();
    CBoundingBox& entityBox = entity.getComponent<CBoundingBox>();
    CFire& entityFire = entity.getComponent<CFire>();

    Vec2f spawnPos = entityTrans.pos + Vec2f(cosf(entityTrans.angle), sinf(entityTrans.angle)) * entityBox.halfSize.x;
    float bulletSpeed = 1.5f; // number of pixels added to bullet on each update

    const sf::Vector2f& worldTargetSFML = m_game.window().mapPixelToCoords(sf::Mouse::getPosition(m_game.window()));
    const Vec2f worldTarget(worldTargetSFML.x, worldTargetSFML.y);
    const Vec2f bulletVec = Vec2f(worldTarget.x - entityTrans.pos.x, worldTarget.y - entityTrans.pos.y).rotate(Random::getFloatingPoint(entityFire.accuracy - 1.f, 1.f - entityFire.accuracy) * static_cast<float>(M_PI));

    Entity bullet = m_entityManager.addEntity("bullet");
    bullet.addComponent<CTransform>(spawnPos, bulletVec * bulletSpeed / (worldTarget - entityTrans.pos).length(), Vec2f(2.0f, 2.0f), bulletVec.angle(), 0.0f);
    bullet.addComponent<CAnimation>(m_game.assets().getAnimation(m_playerConfig.BA), false);
    bullet.addComponent<CLifespan>(300);
    bullet.addComponent<CDamage>(entity.getComponent<CDamage>().damage);

    m_game.assets().playSound("Bullet");
}

/// @brief handle player-tile collisions and player state updates
/// TODO: update for ramp tiles to walk up stairs or hills
void ScenePlay::playerTileCollisions(const std::vector<Tile>& tiles)
{
    PROFILE_FUNCTION();

    CTransform& playerTrans = m_player.getComponent<CTransform>();
    CBoundingBox& playerBounds = m_player.getComponent<CBoundingBox>();
    CState& playerState = m_player.getComponent<CState>();
    // CInput& playerInput = m_player.getComponent<CInput>();

    // bool xCollision = false;
    bool yCollision = false;

    Vec2i playerGridPos = (playerTrans.pos / m_cellSizePixels).to<int>(); // must be signed as subtraction below 0 is happening
    Vec2i checkLength { static_cast<int>(playerBounds.halfSize.x / m_cellSizePixels + 1),
                        static_cast<int>(playerBounds.halfSize.y / m_cellSizePixels + 1) };

    const size_t minX = static_cast<size_t>(std::max(0, playerGridPos.x - checkLength.x));
    const size_t maxX = static_cast<size_t>(std::min(static_cast<int>(m_worldMaxCells.x) - 1, playerGridPos.x + checkLength.x));
    const size_t minY = static_cast<size_t>(std::max(0, playerGridPos.y - checkLength.y));
    const size_t maxY = static_cast<size_t>(std::min(static_cast<int>(m_worldMaxCells.y) - 1, playerGridPos.y + checkLength.y));

    for (size_t x = minX; x <= maxX; ++x)
    {
        for (size_t y = minY; y <= maxY; ++y)
        {
            const Tile& tile = tiles[x * m_worldMaxCells.y + y];

            if (tile.blocksMovement)
            {
                // finding overlap (without tile bounding boxes)
                float xDiff = abs(playerTrans.pos.x - (x + 0.5f) * m_cellSizePixels);
                float yDiff = abs(playerTrans.pos.y - (y + 0.5f) * m_cellSizePixels);
                float xOverlap = playerBounds.halfSize.x + m_cellSizePixels * 0.5f - xDiff;
                float yOverlap = playerBounds.halfSize.y + m_cellSizePixels * 0.5f - yDiff;

                // there is a collision
                if (xOverlap > 0 && yOverlap > 0)
                {
                    // finding previous overlap (without tile bounding boxes)
                    float xPrevDiff = abs(playerTrans.prevPos.x - (x + 0.5f) * m_cellSizePixels);
                    float yPrevDiff = abs(playerTrans.prevPos.y - (y + 0.5f) * m_cellSizePixels);
                    float xPrevOverlap = playerBounds.halfSize.x + m_cellSizePixels * 0.5f - xPrevDiff;
                    float yPrevOverlap = playerBounds.halfSize.y + m_cellSizePixels * 0.5f - yPrevDiff;

                    // we are colliding in y-direction this frame since previous frame already had x-direction overlap
                    if (xPrevOverlap > 0)
                    {
                        yCollision = true;

                        // player moving down
                        if (playerTrans.velocity.y > 0)
                        {
                            playerTrans.pos.y -= yOverlap; // player can't fall below tile
                            playerState.state = abs(playerTrans.velocity.x) > 0 ? State::RUN : State::IDLE;
                        }

                        // player moving up
                        else if (playerTrans.velocity.y < 0)
                        {
                            playerTrans.pos.y += yOverlap;
                        }
                        playerTrans.velocity.y = 0;
                    }
                    // colliding in x-direction this frame
                    else if (yPrevOverlap > 0)
                    {
                        // xCollision = true;

                        // player moving right
                        if (playerTrans.velocity.x > 0)
                        {
                            playerTrans.pos.x -= xOverlap;
                        }
                        // player moving left
                        else if (playerTrans.velocity.x < 0)
                        {
                            playerTrans.pos.x += xOverlap;
                        }
                        playerTrans.velocity.x = 0;
                    }

                    /// TODO: what is neither?

                    break; // found a collision and corrected, no need to do extra work
                }
            }
        }
    }

    if (!yCollision)
    {
        playerState.state = State::AIR;
    }

    // restrict player movement passed top, bottom, or side of map
    const Vec2f& bBoxHalfSize = m_player.getComponent<CBoundingBox>().halfSize;
    if (playerTrans.pos.x < bBoxHalfSize.x)
    {
        playerTrans.pos.x = bBoxHalfSize.x;
        playerTrans.velocity.x = 0;
    }
    else if (playerTrans.pos.x > m_worldMaxPixels.x - bBoxHalfSize.x)
    {
        playerTrans.pos.x = m_worldMaxPixels.x - bBoxHalfSize.x;
        playerTrans.velocity.x = 0;
    }
    else if (playerTrans.pos.y < bBoxHalfSize.y)
    {
        playerTrans.pos.y = bBoxHalfSize.y;
        playerTrans.velocity.y = 0;
    }
    else if (playerTrans.pos.y > m_worldMaxPixels.y - bBoxHalfSize.y)
    {
        playerTrans.pos.y = m_worldMaxPixels.y - bBoxHalfSize.y;
        playerTrans.velocity.y = 0;
    }
}

/// @brief handle bullet-tile collisions
/// TODO: for super fast bullets or just laser tracing whatver it's called, do a line intersect check - easy with tiles since I can just check grid positions along the line from entity to click spot and stop at first intersect
/// TODO: for fast bullets but still projectiles, update the bullet system more than once per game frame
void ScenePlay::projectileTileCollisions(std::vector<Tile>& tiles, std::vector<Entity>& bullets)
{
    PROFILE_FUNCTION();

    // check bullet-tile collisions within a box of size 4 x 4 grid cells around each bullet
    for (const Entity& bullet : bullets)
    {
        CTransform& bulletTrans = bullet.getComponent<CTransform>();

        Vec2i bulletGridPos = (bulletTrans.pos / m_cellSizePixels).to<int>();

        /// TODO: consider adding a bounding box check for bullets (or just leave them as one pixel at the tip of the bullet so I never have to check), depends on what I want with bullet variety (would just have to copy whats in player tiles with bullets)
        // int horizontalCheckLength = bulletBounds.halfSize.x / m_cellSizePixels.x + 1;
        // int verticalCheckLenght = bulletBounds.halfSize.y / m_cellSizePixels.y + 1;
        /// TODO: if no bounding box, don't even need any internal loop, can just get bullet pos (pixels), see if tile active at grid coord, then say there's a collision and handle it, no isInside or overlap or anything either (already done below)

        if (bulletGridPos.x < 0 || bulletGridPos.x >= static_cast<int>(m_worldMaxCells.x) || bulletGridPos.y < 0 || bulletGridPos.y >= static_cast<int>(m_worldMaxCells.y))
        {
            continue;
        }

        Tile& tile = tiles[static_cast<size_t>(bulletGridPos.x) * m_worldMaxCells.y + static_cast<size_t>(bulletGridPos.y)];

        if (tile.blocksMovement) /// TODO: get a blocksProjectiles member? maybe blocks movement but not projectiles, or other way around
        {
            int& bDamage = bullet.getComponent<CDamage>().damage;

            if (bDamage >= tile.health)
            {
                // tile.health = 10;
                // tile.r /= 2;
                // tile.g /= 2;
                // tile.b /= 2;
                // tile.blocksMovement = false;
                // tile.blocksVision = false;
                // if (tile.type == DIRT)
                // {
                //     tile.type = DIRTWALL;
                // }
                // else if (tile.type == STONE)
                // {
                //     tile.type = STONEWALL;
                // }
                // else if (tile.type == BRICK)
                // {
                //     tile.type = BRICKWALL;
                // }
                tile = Tile();
            }
            else
            {
                tile.health -= bDamage;
            }

            bDamage /= 2;

            if (bDamage <= 0)
            {
                bullet.destroy();
            }
            else if (tile.type == TileType::STONE)
            {
                float ricochetChance = Random::getFloatingPoint(0.0f, 1.0f);
                if (ricochetChance > 0.8f)
                {
                    /// TODO: this favors horizontal ricochets a little bit (prevPos is 1.5 pixels behind pos) since horizontal is checked first, could either have more bullet updates to make prvPos and pos closer or use velocity and neighbors and things to make it perfect, but it prolly isn't necessary, or maybe check both and then decide randomly if both work (with x components as below and with y components instead of just an else)

                    if (bulletTrans.prevPos.x > (bulletGridPos.x + 1) * m_cellSizePixels || bulletTrans.prevPos.x < bulletGridPos.x * m_cellSizePixels) // colliding from a side
                    {
                        bulletTrans.velocity.x = -bulletTrans.velocity.x;
                    }
                    else // colliding from the top or bottom
                    {
                        bulletTrans.velocity.y = -bulletTrans.velocity.y;
                    }

                    bulletTrans.angle = -bulletTrans.angle;

                    /// TODO: no richochets after entering solid material, only on surfaces
                    // bullet.canRocichet = false;
                }
            }
        }
        // else
        // {
        //     /// TODO: no richochets after entering solid material, only on surfaces
        //     bullet.canRicochet = true;
        // }

        /// way with the local bounds for checking, if using bounding box for bullets
        // for (int x = bulletGridPosX - 2; x < bulletGridPosX + 2; ++x) /// TODO: tweak these numbers until good
        // {
        //     /// TODO: this may be faster using std::clamp in the loop arguments
        //     if (x < 0 || x >= m_worldMaxCells.x)
        //     {
        //         continue;
        //     }
        //     for (int y = bulletGridPosY - 2; y < bulletGridPosY + 2; ++y)
        //     {
        //         /// TODO: this may be faster using std::clamp in the loop arguments
        //         if (y < 0 || y >= m_worldMaxCells.y)
        //         {
        //             continue;
        //         }
        //         const Entity& tile = tileMatrix[x][y];
        //         if (tile.isActive()) /// TODO: accessing memory in tileMatrix and then switching to entity memory pool, might want local var in tileMatrix or somethin so we don't have to do this
        //         {
        //             // treating bullets as small rectangles to be able to use same Physics::GetOverlap function
        //             Vec2f overlap = Physics::GetOverlap(tileMatrix[x][y], bullet);
        //             if (overlap.x > 0 && overlap.y > 0) // collision happens
        //             {
        //                 int& bDamage = bullet.getComponent<CDamage>().damage;
        //                 int& tHealth = tile.getComponent<CHealth>().current;
        //                 tHealth -= bDamage;
        //                 bDamage /= 2;
        //                 if (bDamage <= 0)
        //                 {
        //                     bullet.destroy();
        //                 }
        //                 /// TODO: could create a toDestory vector and destroy entities all at once somewhere else if faster, test
        //                 if (tHealth <= 0)
        //                 {
        //                     tileMatrix[x][y].destroy();
        //                     /// TODO: check neighbors for floating tiles, then apply physics to them (if no close background)
        //                 }
        //             }
        //         }
        //     }
        // }
    }
}

void ScenePlay::projectilePlayerCollisions(std::vector<Entity>& players, std::vector<Entity>& bullets)
{
    for (Entity& player : players)
    {
        for (Entity& bullet : bullets)
        {
            int& playerInvincibilityTime = player.getComponent<CInvincibility>().timeRemaining;
            Vec2f& playerPos = player.getComponent<CTransform>().pos;
            Vec2f& playerBoxHalfSize = player.getComponent<CBoundingBox>().halfSize;

            if (playerInvincibilityTime <= 0 && Physics::IsInside(bullet.getComponent<CTransform>().pos, playerPos, playerBoxHalfSize))
            {
                playerInvincibilityTime = 10; /// TODO: maybe use another way to keep track of bullets that have already hit the player, make them unable to hit again until leaving the player, could even make no invincibility time and have that be a part of the game, where bullets do more damage the longer they're in the player or a tile, so hitting a leg isn't much compared to hitting a chest (and add a head multiplier), could be a unique aspect to the game
                int& bulletDamage = bullet.getComponent<CDamage>().damage;
                int& playerHealth = player.getComponent<CHealth>().current;
                playerHealth -= bulletDamage;
                bulletDamage /= 2; /// TODO: tweak later

                if (bulletDamage <= 0)
                {
                    bullet.destroy();
                }

                if (playerHealth <= 0)
                {
                    createRagdoll(player, bullet);
                    player.destroy();

                    createRagdoll(m_weapon, bullet); /// TODO: don't use m_weapon, use something else maybe
                    m_weapon.destroy();
                }
            }
        }
    }
}

/// @brief replace entity with ragdoll version created when cause kills entity
Entity ScenePlay::spawnRagdollElement(const Vec2f& pos, float angle, const Vec2i& boxSize, const Animation& animation)
{
    Entity ragdoll = m_entityManager.addEntity("ragdoll");
    ragdoll.addComponent<CTransform>(pos, angle);
    ragdoll.addComponent<CGravity>(m_playerConfig.GRAVITY);
    ragdoll.addComponent<CBoundingBox>(boxSize);
    ragdoll.addComponent<CAnimation>(animation, false);
    ragdoll.addComponent<CLifespan>(600);

    return ragdoll;
}

void ScenePlay::createRagdoll(const Entity& entity, const Entity& cause)
{
    const CTransform& entityTrans = entity.getComponent<CTransform>();
    const CBoundingBox& entityBox = entity.getComponent<CBoundingBox>();
    const CAnimation& entityAnim = entity.getComponent<CAnimation>();
    const CTransform& causeTrans = cause.getComponent<CTransform>();

    // weapon
    if (entity.hasComponent<CFire>())
    {
        Entity ragdoll = spawnRagdollElement(entityTrans.pos, entityTrans.angle, entityBox.size, entityAnim.animation);
        CTransform& ragTrans = ragdoll.getComponent<CTransform>();
        CBoundingBox& ragBox = ragdoll.getComponent<CBoundingBox>();

        Vec2f force;
        Vec2f pos;
        force.x = (causeTrans.velocity.x + Random::getFloatingPoint(0.0f, causeTrans.velocity.length())) * 2.0f;
        force.y = (causeTrans.velocity.y + Random::getFloatingPoint(0.0f, causeTrans.velocity.length())) * 2.0f;
        pos.x = entityTrans.pos.x + Random::getFloatingPoint(0.0f, entityBox.halfSize.x);
        pos.y = entityTrans.pos.y + Random::getFloatingPoint(0.0f, entityBox.halfSize.y);

        Physics::ForceEntity(ragTrans.pos, ragTrans.velocity, ragTrans.angularVelocity, ragBox.size, force, pos);
    }
    else // entity is a player
    {
        /// TODO: change to correct animations, angles, and positions, and make sizing dynamic
        // created in order of rendering
        const Animation& tempTest = m_game.assets().getAnimation("Test");
        Entity backForearm = spawnRagdollElement(entityTrans.pos, 0.0f, { 6, 12 }, tempTest);
        Entity backUpperArm = spawnRagdollElement(entityTrans.pos, 0.0f, { 6, 12 }, tempTest);
        Entity backCalf = spawnRagdollElement(entityTrans.pos, 0.0f, { 6, 22 }, tempTest);
        Entity backThigh = spawnRagdollElement(entityTrans.pos, 0.0f, { 6, 15 }, tempTest);
        Entity torso = spawnRagdollElement(entityTrans.pos, 0.0f, { 6, 30 }, tempTest);
        Entity head = spawnRagdollElement(entityTrans.pos, 0.0f, { 6, 8 }, tempTest);
        Entity frontCalf = spawnRagdollElement(entityTrans.pos, 0.0f, { 6, 22 }, tempTest);
        Entity frontThigh = spawnRagdollElement(entityTrans.pos, 0.0f, { 6, 15 }, tempTest);
        Entity frontForearm = spawnRagdollElement(entityTrans.pos, 0.0f, { 6, 12 }, tempTest);
        Entity frontUpperArm = spawnRagdollElement(entityTrans.pos, 0.0f, { 6, 12 }, tempTest);

        std::array<float, 3> positions({ 0.0f, 0.0f, 0.0f });

        // angles defined for when player dies facing right /// TODO: flip them in sObjectMovement if player dies facing left
        head.addComponent<CJointRelation>(torso, -Constants::pi / 6.0f, Constants::pi / 4.0f);
        backThigh.addComponent<CJointRelation>(torso, -Constants::pi / 4.0f, 3.0f * Constants::pi / 4.0f);
        frontThigh.addComponent<CJointRelation>(torso, -Constants::pi / 4.0f, 3.0f * Constants::pi / 4.0f);
        backUpperArm.addComponent<CJointRelation>(torso, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        frontUpperArm.addComponent<CJointRelation>(torso, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        backCalf.addComponent<CJointRelation>(backThigh, 0.0f, 5.0f * Constants::pi / 6.0f);
        frontCalf.addComponent<CJointRelation>(frontThigh, 0.0f, 5.0f * Constants::pi / 6.0f);
        backForearm.addComponent<CJointRelation>(backUpperArm, 0.0f, 5.0f * Constants::pi / 6.0f);
        frontForearm.addComponent<CJointRelation>(frontUpperArm, 0.0f, 5.0f * Constants::pi / 6.0f);

        // add joint positions
        positions[0] = head.getComponent<CBoundingBox>().halfSize.y; // to torso
        head.addComponent<CJointInfo>(positions);

        positions[0] = -backForearm.getComponent<CBoundingBox>().halfSize.y; // to back upper arm
        backForearm.addComponent<CJointInfo>(positions);

        positions[0] = -frontForearm.getComponent<CBoundingBox>().halfSize.y; // to front upper arm
        frontForearm.addComponent<CJointInfo>(positions);

        positions[0] = -backCalf.getComponent<CBoundingBox>().halfSize.y; // to back thigh
        backCalf.addComponent<CJointInfo>(positions);

        positions[0] = -frontCalf.getComponent<CBoundingBox>().halfSize.y; // to front thigh
        frontCalf.addComponent<CJointInfo>(positions);

        CBoundingBox& ltb = backThigh.getComponent<CBoundingBox>();
        positions[0] = ltb.halfSize.y; // to back calf
        positions[1] = -ltb.halfSize.y; // to torso
        backThigh.addComponent<CJointInfo>(positions);

        CBoundingBox& rtb = frontThigh.getComponent<CBoundingBox>();
        positions[0] = rtb.halfSize.y; // to front calf
        positions[1] = -rtb.halfSize.y; // to torso
        frontThigh.addComponent<CJointInfo>(positions);

        CBoundingBox& luab = backUpperArm.getComponent<CBoundingBox>();
        positions[0] = luab.halfSize.y; // to back forearm
        positions[2] = -luab.halfSize.y; // to torso
        backUpperArm.addComponent<CJointInfo>(positions);

        CBoundingBox& ruab = frontUpperArm.getComponent<CBoundingBox>();
        positions[0] = ruab.halfSize.y; // to front forearm
        positions[2] = -ruab.halfSize.y; // to torso
        frontUpperArm.addComponent<CJointInfo>(positions);

        CBoundingBox& tb = torso.getComponent<CBoundingBox>();
        positions[0] = -tb.halfSize.y; // to head
        positions[1] = tb.halfSize.y * 0.8f; // to thighs
        positions[2] = -tb.halfSize.y * 0.8f; // to upper arms
        torso.addComponent<CJointInfo>(positions);

        // apply initial force to correct place from cause
        // if (causeTrans.pos.y <= entityTrans.pos.y) // hit top half of body
        // {
        //     Physics::ForceEntity(head, causeTrans.velocity * 10.0f, causeTrans.pos); // arbitrary choice of applied force
        // }
        // else
        // {
        //     Physics::ForceEntity(torso, causeTrans.velocity * 10.0f, causeTrans.pos);
        // }

        CTransform& tt = torso.getComponent<CTransform>();
        Physics::ForceEntity(tt.pos, tt.velocity, tt.angularVelocity, tb.size, causeTrans.velocity * 10.0f * Random::getFloatingPoint(0.5f, 2.0f), causeTrans.pos);
    }
}

/// @brief move all projectiles and check for collisions
void ScenePlay::updateProjectiles(std::vector<Entity>& projectiles)
{
    PROFILE_FUNCTION();

    /// TODO: works for bullets, change when adding more projectile types
    for (Entity& projectile : projectiles)
    {
        // movement
        CTransform& projectileTrans = projectile.getComponent<CTransform>();
        projectileTrans.prevPos = projectileTrans.pos;
        projectileTrans.pos += projectileTrans.velocity;

        // collisions with players
        // for (Entity& player : m_entityManager.getEntities("player"))
        // {
        //     if (Physics::IsInside(projectileTrans.pos, player) && player.getComponent<CInvincibility>().timeRemaining > 0)
        //     {
        //         int& projectileDamage = projectile.getComponent<CDamage>().damage;
        //         int& playerHealth = player.getComponent<CHealth>().current;
        //         playerHealth -= projectileDamage;
        //         projectileDamage /= 2; /// TODO: tweak later

        //         if (projectileDamage <= 0)
        //         {
        //             projectile.destroy();
        //         }

        //         if (playerHealth <= 0)
        //         {
        //             std::cout << "killed player" << std::endl;
        //             player.destroy();
        //         }
        //     }
        // }
    }

    std::vector<Entity>& players = m_entityManager.getEntities("player");

    // move other existing projectiles (like bombs, affected by gravity)
    /// TODO: remember to group these checks so that it's fast, might want to use "projectile" tag in entity manager and use an if (hasComponent(<CGravity>)) or whatever to find the bombs vs bullets vs whatever, all in one loop
    /// TODO: be more ECS-like, put all manip of CTrans in the sMovement system or something

    // check for collisions with tiles
    std::vector<Tile>& tiles = m_tileManager.getTiles();
    projectileTileCollisions(tiles, projectiles);
    projectilePlayerCollisions(players, projectiles);
}

/// @brief find tile grid coords that are reachable from (x, y) grid coords without breaking other tiles and add them to openTiles
/// TODO: way to only update visible tiles on certain events like destroy tile, move, place tile, etc.?
void ScenePlay::findOpenTiles(size_t x, size_t y, size_t minX, size_t maxX, size_t minY, size_t maxY, const std::vector<Tile>& tiles, std::vector<Vec2i>& openTiles, std::stack<Vec2i>& tileStack, std::vector<std::vector<bool>>& visited)
{
    // base case - off game world or rendering screen
    /// TODO: seg fault on world edge case, just make world bounds so that camera always in middle and player never reaches "edge"
    if (x < minX || y < minY || x > maxX || y > maxY)
    {
        return;
    }

    // base case - active tile found
    const Tile& tile = tiles[x * m_worldMaxCells.y + y];
    if (tile.blocksVision)
    {
        openTiles.emplace_back(x, y);
        return;
    }

    // recursive step - add all neighbors and call function on next tile
    if (x < maxX && !visited[x - minX + 1][y - minY])
    {
        tileStack.emplace(x + 1, y);
        visited[x - minX + 1][y - minY] = true;
    }
    if (y > minY && !visited[x - minX][y - minY - 1])
    {
        tileStack.emplace(x, y - 1);
        visited[x - minX][y - minY - 1] = true;
    }
    if (x > minX && !visited[x - minX - 1][y - minY])
    {
        tileStack.emplace(x - 1, y);
        visited[x - minX - 1][y - minY] = true;
    }
    if (y < maxY && !visited[x - minX][y - minY + 1])
    {
        tileStack.emplace(x, y + 1);
        visited[x - minX][y - minY + 1] = true;
    }

    while (!tileStack.empty())
    {
        Vec2uz topVal = tileStack.top().to<size_t>();
        tileStack.pop();
        findOpenTiles(topVal.x, topVal.y, minX, maxX, minY, maxY, tiles, openTiles, tileStack, visited);
    }
}

std::vector<Vec2f> ScenePlay::rayCast(const Vec2f& viewCenter, const Vec2f& viewSize, const std::vector<Vec2i>& openTiles, const Vec2f& origin, const std::vector<Tile>& tiles, size_t minX, size_t maxX, size_t minY, size_t maxY)
{
    PROFILE_FUNCTION();

    // use open-air tiles with ray casting
    std::vector<Vec2f> vertices; // in pixels
    std::unordered_set<Vec2f> vertexSet; // use unordered set to collect unique vertices and put only unique ones in vector

    // four corners of the screen
    /// TODO: see if the + 1 is necessary now that they're floats
    vertices.emplace_back(viewCenter.x - viewSize.x / 2.0f, viewCenter.y - viewSize.y / 2.0f);
    vertices.emplace_back(viewCenter.x + viewSize.x / 2.0f, viewCenter.y - viewSize.y / 2.0f);
    vertices.emplace_back(viewCenter.x - viewSize.x / 2.0f, viewCenter.y + viewSize.y / 2.0f); /// TODO: these may not be necessary of players will never reach bottom of world
    vertices.emplace_back(viewCenter.x + viewSize.x / 2.0f, viewCenter.y + viewSize.y / 2.0f); /// TODO: these may not be necessary of players will never reach bottom of world

    // add open tile corners
    for (const Vec2i& tileCoords : openTiles)
    {
        Vec2i corners[4] = { tileCoords,
                           { tileCoords.x, tileCoords.y + 1 },
                           { tileCoords.x + 1, tileCoords.y },
                           { tileCoords.x + 1, tileCoords.y + 1 } };

        for (const Vec2i& v : corners)
        {
            if (vertexSet.insert((v * m_cellSizePixels).to<float>()).second) // insert() returns {iterator, bool}, bool is true if inserted
            {
                vertices.push_back((v * m_cellSizePixels).to<float>());
            }
        }
    }

    // filter visible vertices and add new points behind them
    std::vector<Vec2f> verticesToAdd;
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        Vec2f& vertex = vertices[i];
        float rayAngle = (vertex - origin).angle();

        /// for this method: if it's middle ray and reaches vertex, remove it; if it's side ray, let it go until it hits something and add a vertex there
        // for (int dTheta = -1; dTheta < 2; ++dTheta)
        // {
            // float angle = rayAngle + dTheta * 0.0001f;
        float angle = rayAngle;
        Vec2f rayUnitVec(cosf(angle), sinf(angle));
        float slope = tanf(angle);
        float reciprocalSlope = 1.0f / slope;
        float xMoveHypDist = sqrtf(1 + slope * slope); // in grid coords
        float yMoveHypDist = sqrtf(1 + reciprocalSlope * reciprocalSlope); // in grid coords

        // coordinates of grid cell the traveling ray is in
        /// TODO: test edge cases: if hitting right side of tile, I want this coord to truncate to the left side of the tile (since tiles positioned with top-left), this could be wrong, subtly
        Vec2i gridCoord = origin.to<int>() / m_cellSizePixels;

        // way we step in x or y when traveling the ray
        Vec2i rayStep;

        // accumulated distances (pixels) in the direction of the hypoteneuse caused by a change in x/y from the start of the ray (player position), starting with initial pixel offset from grid coord (top-left)
        float xTravel, yTravel;

        if (rayUnitVec.x < 0)
        {
            rayStep.x = -1;
            xTravel = (origin.x - (gridCoord.x * m_cellSizePixels)) * xMoveHypDist;
        }
        else
        {
            rayStep.x = 1;
            xTravel = (m_cellSizePixels - (origin.x - (gridCoord.x * m_cellSizePixels))) * xMoveHypDist;
        }

        if (rayUnitVec.y < 0)
        {
            rayStep.y = -1;
            yTravel = (origin.y - (gridCoord.y * m_cellSizePixels)) * yMoveHypDist;
        }
        else
        {
            rayStep.y = 1;
            yTravel = (m_cellSizePixels - (origin.y - (gridCoord.y * m_cellSizePixels))) * yMoveHypDist;
        }

        bool tileHit = false;
        bool vertexReached = false;
        // bool topLeft, topRight, bottomLeft, bottomRight;
        while (!tileHit && !vertexReached) /// TODO: alternatively, add a xTravel < rayLength - 0.001f or something instead of vertexReached
        {
            if (xTravel < yTravel)
            {
                gridCoord.x += rayStep.x;

                // at this point, the endpoint of the line formed by origin + xTravel * rayUnitVec is the collision point on tile (gridCoord.x, gridCoord.y) if tile is active there

                xTravel += xMoveHypDist * m_cellSizePixels;
            }
            else
            {
                gridCoord.y += rayStep.y;
                yTravel += yMoveHypDist * m_cellSizePixels;
            }

            const Tile& tile = tiles[static_cast<size_t>(gridCoord.x) * m_worldMaxCells.y + static_cast<size_t>(gridCoord.y)];
            if (tile.blocksVision) // tile hit /// TODO: seg fault on edges of world or if second check fails and this goes on
            {
                tileHit = true;

                // remove vertex from vertices
                vertex = vertices.back();
                vertices.pop_back();
                --i;
            }

            // if vertex reached and not originally unique (another tile shares it so don't go through) just keep vertex point, do nothing
            /// TODO: keep repeats? given this thought above ^
            if (!tile.blocksVision && (vertex.x >= gridCoord.x * m_cellSizePixels && vertex.x <= (gridCoord.x + 1) * m_cellSizePixels && vertex.y >= gridCoord.y * m_cellSizePixels && vertex.y <= (gridCoord.y + 1) * m_cellSizePixels)) // vertex reached since in cell with no tile but with this vertex
            {
                vertexReached = true;
            }
        }
        // }

        /// find vertex and divert angle method, less rays, but possibly still more work
        /// @todo try the 3 ray method since branch prediction would be better without all the if's
        // note that the top two corners of the screen are reached as well as actual vertices
        if (vertexReached)
        {
            /// if vertex reached and if just passed there is no tile, expand line to next intersection or end of screen and create new point there for triangle fan

            // check for tile just passed the vertex in the direction of the ray
            Vec2uz checkCoord = ((vertex + rayUnitVec) / m_cellSizePixels).to<size_t>();

            const Tile& tile = tiles[checkCoord.x * m_worldMaxCells.y + checkCoord.y];
            if (!(checkCoord.x < minX || checkCoord.x > maxX || checkCoord.y < minY || checkCoord.y > maxY || tile.blocksVision))
            {
                /// add small angle to ray
                /// NOTE: angle starts at 0 on +x-axis and increases in a CW manner up to 2π (since +y-axis points down)
                /// TODO: is this more expensive than just adding two more rays at slight angle offsets for each ray?
                /// TODO: handle pure vertical and horizontal rays case

                float dTheta = 0.0001f;
                float newAngle;
                Vec2uz gridCoordUZ = gridCoord.to<size_t>();

                if (rayUnitVec.x < 0 && rayUnitVec.y < 0) // came from bottom right
                {
                    /// @todo ensure gridCoord > (0, 0) or we'll get size_t wrap around
                    if (tiles[(gridCoordUZ.x - 1) * m_worldMaxCells.y + gridCoordUZ.y].blocksVision && tiles[gridCoordUZ.x * m_worldMaxCells.y + gridCoordUZ.y - 1].blocksVision) // shared vertex
                        continue; // don't want ray shining through diagonal lines of tiles

                    if (tiles[(gridCoordUZ.x - 1) * m_worldMaxCells.y + gridCoordUZ.y].blocksVision) // tile to the left active
                        newAngle = rayAngle + dTheta; // CW
                    else // tile above active
                        newAngle = rayAngle - dTheta; // CCW
                }
                else if (rayUnitVec.x > 0 && rayUnitVec.y < 0) // came from bottom left
                {
                    if (tiles[gridCoordUZ.x * m_worldMaxCells.y + (gridCoordUZ.y - 1)].blocksVision && tiles[(gridCoordUZ.x + 1) * m_worldMaxCells.y + gridCoordUZ.y].blocksVision)
                        continue;

                    if (tiles[gridCoordUZ.x * m_worldMaxCells.y + (gridCoordUZ.y - 1)].blocksVision) // tile above active
                        newAngle = rayAngle + dTheta; // CW
                    else // tile to the right active
                        newAngle = rayAngle - dTheta;; // CCW
                }
                else if (rayUnitVec.x > 0 && rayUnitVec.y > 0) // came from top left
                {
                    if (tiles[(gridCoordUZ.x + 1) * m_worldMaxCells.y + gridCoordUZ.y].blocksVision && tiles[gridCoordUZ.x * m_worldMaxCells.y + (gridCoordUZ.y + 1)].blocksVision)
                        continue;

                    if (tiles[(gridCoordUZ.x + 1) * m_worldMaxCells.y + gridCoordUZ.y].blocksVision) // tile to the right
                        newAngle = rayAngle + dTheta; // CW
                    else // tile below is active
                        newAngle = rayAngle - dTheta; // CCW
                }
                else // came from top right or /// TODO: a horizontal/vertical ray case
                {
                    if (tiles[gridCoordUZ.x * m_worldMaxCells.y + (gridCoordUZ.y + 1)].blocksVision && tiles[(gridCoordUZ.x - 1) * m_worldMaxCells.y + gridCoordUZ.y].blocksVision)
                        continue;

                    if (tiles[gridCoordUZ.x * m_worldMaxCells.y + (gridCoordUZ.y + 1)].blocksVision) // tile below is active
                        newAngle = rayAngle + dTheta; // CW
                    else // tile to the left is active
                        newAngle = rayAngle - dTheta; // CCW
                }

                Vec2f newRayUnitVec = { cosf(newAngle), sinf(newAngle) };
                slope = tanf(newAngle);
                reciprocalSlope = 1.0f / slope;
                xMoveHypDist = sqrtf(1 + slope * slope);
                yMoveHypDist = sqrtf(1 + reciprocalSlope * reciprocalSlope);

                float xTravelNew = xMoveHypDist * m_cellSizePixels;
                float yTravelNew = yMoveHypDist * m_cellSizePixels;
                gridCoord = checkCoord.to<int>();

                while (!tileHit)
                {
                    if (xTravelNew < yTravelNew)
                    {
                        gridCoord.x += rayStep.x;

                        // at this point, the endpoint of the line formed by vertex + xTravelNew * newRayUnitVec is the collision point on tile (gridCoord.x, gridCoord.y) if tile is active there

                        gridCoordUZ = gridCoord.to<size_t>();
                        if (gridCoordUZ.x < minX || gridCoordUZ.x > maxX || tiles[gridCoordUZ.x * m_worldMaxCells.y + gridCoordUZ.y].blocksVision)
                        {
                            tileHit = true;
                            verticesToAdd.push_back(vertex + newRayUnitVec * xTravelNew);
                        }
                        else
                        {
                            xTravelNew += xMoveHypDist * m_cellSizePixels;
                        }
                    }
                    else
                    {
                        gridCoord.y += rayStep.y;

                        gridCoordUZ = gridCoord.to<size_t>();
                        if (gridCoordUZ.y < minY || gridCoordUZ.y > maxY || tiles[gridCoordUZ.x * m_worldMaxCells.y + gridCoordUZ.y].blocksVision)
                        {
                            tileHit = true;
                            verticesToAdd.push_back(vertex + newRayUnitVec * yTravelNew);
                        }
                        else
                        {
                            yTravelNew += yMoveHypDist * m_cellSizePixels;
                        }
                    }
                }
            }
        }
    }

    // for (const auto& v : verticesToAdd)
    // {
    //     sf::CircleShape dot(2);
    //     dot.setPosition({ v.x - 2, v.y - 2 });
    //     dot.setFillColor(sf::Color::Red);
    //     window.draw(dot);
    // }
    vertices.insert(vertices.end(), verticesToAdd.begin(), verticesToAdd.end());

    // sort the reachable vertices by angle
    /// TODO: consider storing angle for each vertex in the vector, faster probably
    std::sort(vertices.begin(), vertices.end(), [&origin](const Vec2f& a, const Vec2f& b) { return (origin - a).angle() < (origin - b).angle(); });

    // create triangle fan of vertices (pixels)
    std::vector<Vec2f> triangleFan;
    triangleFan.push_back(origin);
    triangleFan.insert(triangleFan.end(), vertices.begin(), vertices.end());
    triangleFan.push_back(vertices.front());

    return triangleFan;
}

/// TODO: memory leak or something in this scope causes game to get real slow after about 40 seconds
// void ScenePlay::propagateLight(sf::VertexArray& blocks, int maxDepth, int currentDepth, const Vec2i& startCoord, Vec2i currentCoord, int minX, int maxX, int minY, int maxY)
// {
//     // base case
//     if (currentDepth >= maxDepth)
//     {
//         return;
//     }

//     // recursive step
//     Tile& tile = m_tileManager.getTiles()[currentCoord.x * m_worldMaxCells.y + currentCoord.y];
//     std::cout << "Processing tile at " << currentCoord << "\n";

//     if (tile.health)
//     {
//         int newLight = 255 - currentDepth * (255 / maxDepth);
//         std::cout << "    light: " << static_cast<int>(tile.light) << ", newLight: " << newLight << std::endl;
//         if (tile.light < newLight)
//         {
//             tile.light = static_cast<uint8_t>(newLight);

//             // create block with 2 triangles
//             sf::Color c(tile.r, tile.g, tile.b, tile.light);
//             addBlock(blocks, currentCoord.x, currentCoord.y, c);

//             std::cout << "        Depth: " << currentDepth << std::endl;

//             Vec2i playerGridPos = (m_player.getComponent<CTransform>().pos / m_cellSizePixels).to<int>();
//             if (currentCoord.x > minX && playerGridPos.x >= currentCoord.x)
//             {
//                 propagateLight(blocks, maxDepth, currentDepth + 1, startCoord, Vec2i(currentCoord.x - 1, currentCoord.y), minX, maxX, minY, maxY);
//             }
//             if (currentCoord.x < maxX && playerGridPos.x <= currentCoord.x)
//             {
//                 propagateLight(blocks, maxDepth, currentDepth + 1, startCoord, Vec2i(currentCoord.x + 1, currentCoord.y), minX, maxX, minY, maxY);
//             }
//             if (currentCoord.y > minY && playerGridPos.y >= currentCoord.y)
//             {
//                 propagateLight(blocks, maxDepth, currentDepth + 1, startCoord, Vec2i(currentCoord.x, currentCoord.y - 1), minX, maxX, minY, maxY);
//             }
//             if (currentCoord.y < maxY && playerGridPos.y <= currentCoord.y)
//             {
//                 propagateLight(blocks, maxDepth, currentDepth + 1, startCoord, Vec2i(currentCoord.x, currentCoord.y + 1), minX, maxX, minY, maxY);
//             }
//         }
//     }

//     return;
// }

void ScenePlay::addBlock(sf::VertexArray& blocks, int xGrid, int yGrid, const sf::Color& c)
{
    float px = xGrid * m_cellSizePixels;
    float py = yGrid * m_cellSizePixels;

    blocks.append({ { px, py }, c });
    blocks.append({ { px + m_cellSizePixels, py }, c });
    blocks.append({ { px, py + m_cellSizePixels }, c });
    blocks.append({ { px, py + m_cellSizePixels }, c });
    blocks.append({ { px + m_cellSizePixels, py }, c });
    blocks.append({ { px + m_cellSizePixels, py + m_cellSizePixels }, c });
}
