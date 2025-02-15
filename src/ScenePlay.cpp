/// TODO: multithreading
/// TODO: worry about signed vs unsigned and size and even size_t (adapts to platform's word size) and all that later after learning more about performance differences and such (e.g., may not want to mix signed and unsigned ints like uint32_t and int8_t)
/// TODO: in some cases processing a 64-bit int is faster than a 32-bit one (or 32 faster than 16), but in many cases memory is what slows down a program, so just have to test between memory efficiency and CPU efficiency
/// TODO: render tile layer (and any other things) at the minimum resolution in it's own view, then scale the size of that view to match with others (this way the resolution of the tile map can be shit (and minimap) but still look the same, and the character and guns and all can be great, can even put back in textures for tiles probably)

#include "Timer.hpp"
#include "Globals.hpp"

#include "ScenePlay.hpp"
#include "Scene.hpp"
#include "GameEngine.hpp"
#include "EntityManager.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include "Vec2.hpp"
#include "Physics.hpp"
#include "Animation.hpp"
#include "Action.hpp"
#include "WorldGenerator.hpp"
#include "TileType.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <chrono>
#include <random> // number generation for colors of blocks
#include <unordered_set>

/// @brief vonstructs a new ScenePlay object, calls ScenePlay::init
/// @param gameEngine the game's main engine which handles scene switching and adding, and other top-level functions; required by Scene to set m_game
ScenePlay::ScenePlay(GameEngine& gameEngine)
    : Scene(gameEngine)
{
    PROFILE_FUNCTION();

    init();
}

/**
 * scene initialization functions
 */

 /// @brief initializes the scene: registers keybinds, sets grid and fps text attributes, and calls loadGame
void ScenePlay::init()
{
    PROFILE_FUNCTION();

    // misc keybind setup
    registerAction(static_cast<int>(sf::Keyboard::Key::P), "PAUSE");
    registerAction(static_cast<int>(sf::Keyboard::Key::Escape), "QUIT");
    registerAction(static_cast<int>(sf::Keyboard::Key::T), "TOGGLE_TEXTURE");
    registerAction(static_cast<int>(sf::Keyboard::Key::C), "TOGGLE_COLLISION");
    registerAction(static_cast<int>(sf::Keyboard::Key::M), "TOGGLE_MAP");

    // player keyboard setup
    registerAction(static_cast<int>(sf::Keyboard::Key::W), "JUMP");
    registerAction(static_cast<int>(sf::Keyboard::Key::A), "LEFT");
    registerAction(static_cast<int>(sf::Keyboard::Key::D), "RIGHT");

    // player mouse setup
    registerAction(static_cast<int>(sf::Mouse::Button::Left), "SHOOT", true);

    // fps counter setup
    m_fpsText.setCharacterSize(12);
    m_fpsText.setFillColor(sf::Color::White);
    m_fpsText.setPosition({ 10.f, 10.f }); // top-left corner

    // minimap setup
    m_miniMapView.setViewport(sf::FloatRect({ 0.75f, 0.1f }, { 0.2f, 0.3556f }));

    loadGame();
}

/// TODO: consider keeping this for later in case people want to save level, then can load with this function
/// @brief loads the scene
void ScenePlay::loadGame()
{
    PROFILE_FUNCTION();

    // read in the level file and add the appropriate entities
    std::ifstream file("../bin/playerConfig.txt");

    if (!file.is_open())
    {
        std::cerr << "Player file could not be opened: " << "../bin/playerConfig.txt" << std::endl;
        exit(-1);
    }

    std::string type;

    while (file >> type)
    {
        /// TODO: consider position decorations w.r.t. their top-left corner using gridToPixel instead of center (things with collisions implemented with centered positions)
        if (type == "Tile")
        {
            // std::shared_ptr<Entity> tile = m_entityManager.addEntity("tile");

            // // add CAnimation component first so that gridToMidPixel works
            // std::string animation;
            // file >> animation;
            // tile->add<CAnimation>(m_game.assets().getAnimation(animation), true);

            // float gridX, gridY;
            // file >> gridX >> gridY;
            // tile->add<CTransform>(gridToMidPixel(gridX, gridY, tile));

            // tile->add<CBoundingBox>(m_game.assets().getAnimation(animation).getSize());
        }
        else if (type == "Dec")
        {

        }
        else if (type == "Player")
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

    generateWorld();

    spawnPlayer();
}

/// @brief randomly generate the playing world
void ScenePlay::generateWorld()
{
    PROFILE_FUNCTION();

    /// TODO: mountains, caves, lakes, rivers, even terrain, biomes, etc.
    /// TODO: consider different noise types for speed

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

    // generate world and get tile positions
    WorldGenerator gen(m_worldMaxCells.x, m_worldMaxCells.y);
    gen.generateWorld();
    const std::vector<std::vector<TileType>>& tileMatrix = gen.getTileMatrix();

    // spawn tiles according to their positions in the grid
    for (int x = 0; x < m_worldMaxCells.x; ++x)
    {
        // PROFILE_SCOPE("adding tiles, row x");

        for (int y = 0; y < m_worldMaxCells.y; ++y)
        {
            TileType tileType = tileMatrix[x][y];
            if (tileType)
            {
                // std::cout << "adding tile " << x << ", " << y << std::endl;

                Entity tile = m_entityManager.addEntity("tile");

                // tile.addComponent<CAnimation>(m_game.assets().getAnimation(tileMatrix[x][y]), true); /// TODO: this is what takes so long to gen world, adding animation, seems like this loop just stops (think twice now) at entity 499399
                // tile.addComponent<CTransform>(gridToMidPixel(x, y, tile));
                // tile.addComponent<CBoundingBox>(GlobalSettings::cellSizePixels, true, true);
                // tile.addComponent<CPosition>(gridToMidPixel(x, y, tile)); // uses CBoundingBox data, must be after
                tile.addComponent<CType>(tileType);

                float randomNumber = generateRandomFloat(0.9f, 1.1f); /// TODO: could be faster to use predetermined values or formula, like function of depth or something

                if (tileType == DIRT)
                {
                    tile.addComponent<CHealth>(40);
                    tile.addComponent<CColor>(100.0f * randomNumber, 60.0f * randomNumber, 40.0f * randomNumber);
                }
                else if (tileType == STONE)
                {
                    tile.addComponent<CHealth>(60);
                    tile.addComponent<CColor>(145.0f * randomNumber, 145.0f * randomNumber, 145.0f * randomNumber);
                }
                else
                {
                    std::cerr << "invalid tile type\n";
                    exit(-1);
                }

                m_entityManager.addTileToMatrix(tile, x, y);
            }
        }
    }
}

/**
 * scene management functions
 */

 /// @brief update the scene; this function is called by the game engine at each frame if this scene is active
void ScenePlay::updateState(std::chrono::duration<long long, std::nano>& lag)
{
    PROFILE_FUNCTION();

    if (!m_paused)
    {
        while (lag >= std::chrono::duration<long long, std::nano>(1000000000 / GlobalSettings::frameRate)) /// TODO: consider using doubles or something to be more precise with timing, or just longs to be smaller in memory
        {
            // this can be infinite loop if it takes longer to do all this than the time per frame
            /// TODO: think about order here if it even matters
            sStatus(); // lifespan and invincibility time calculations first to not waste calculations on dead entities
            sObjectMovement(); // object movement
            sObjectCollision(); // then object collisions
            sProjectiles(); // then iterations of projectile movement and collisions, then projectile spawns
            sAI();
            sAnimation(); // update all animations (could move this around)
            sCamera(); // finally, set camera

            m_entityManager.update(); // add and remove all entities staged during updates above

            lag -= std::chrono::duration<long long, std::nano>(1000000000 / GlobalSettings::frameRate); /// TODO: will rounding be an issue here?
        }
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

    float airResistance = 15.0f; // m/s slow-down

    // player
    if (m_player.isActive())
    {
        std::string& playerState = m_player.getComponent<CState>().state;
        CInput& playerInput = m_player.getComponent<CInput>();
        CTransform& playerTrans = m_player.getComponent<CTransform>();
        CGravity& playerGrav = m_player.getComponent<CGravity>();

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
            if (playerState == "air")
            {
                friction = 0.2f;
            }
            else // if (playerState == "run")
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

        if (playerInput.up && playerInput.canJump)
        {
            velToAdd.y -= m_playerConfig.SY;
            playerInput.canJump = false; // set to true in sCollision (must see if on the ground)
        }

        // on release of jump key
        /// TODO: implement new jumping (min jump height, no sudden fall on release, double jumping / flying)
        if (!playerInput.up && playerTrans.velocity.y < 0)
        {
            playerTrans.velocity.y = 0;
        }

        playerTrans.velocity += velToAdd;
        playerTrans.prevPos = playerTrans.pos;
        playerTrans.pos += playerTrans.velocity;

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

        // set scale based on mouse position
        const Vec2f& worldTarget = m_game.window().mapPixelToCoords(sf::Mouse::getPosition(m_game.window()));
        const Vec2f aimVec = worldTarget - playerTrans.pos;
        if (aimVec.x < 0)
        {
            playerTrans.scale.x = -abs(playerTrans.scale.x);
        }
        else
        {
            playerTrans.scale.x = abs(playerTrans.scale.x);
        }

        // weapon held by player
        for (Entity& weapon : m_entityManager.getEntities("weapon"))
        {
            CTransform& weaponTrans = weapon.getComponent<CTransform>();
            CBoundingBox& weaponBox = weapon.getComponent<CBoundingBox>();

            weaponTrans.pos = playerTrans.pos + aimVec.norm() * weaponBox.halfSize.x;
            weaponTrans.angle = aimVec.angle();

            // set scale based on mouse position
            if (aimVec.x < 0)
            {
                weaponTrans.scale.y = -abs(weaponTrans.scale.x);
            }
            else
            {
                weaponTrans.scale.y = abs(weaponTrans.scale.x);
            }
        }
    }

    // ragdolls
    for (Entity& radgoll : m_entityManager.getEntities("ragdoll"))
    {
        CTransform& ragTrans = radgoll.getComponent<CTransform>();
        CBoundingBox& ragBox = radgoll.getComponent<CBoundingBox>();
        CGravity& ragGrav = radgoll.getComponent<CGravity>();

        if (ragTrans.velocity.y + ragGrav.gravity >= airResistance)
        {
            ragTrans.velocity.y += airResistance - ragTrans.velocity.y;
        }
        else
        {
            ragTrans.velocity.y += ragGrav.gravity;
        }

        ragTrans.prevPos = ragTrans.pos;
        ragTrans.pos += ragTrans.velocity;
        ragTrans.prevAngle = ragTrans.angle;
        ragTrans.angle += ragTrans.angularVelocity;
    }
}

/// TODO: modularize some of this if needed to reduce repition and make it easier to read
/// TODO: increase efficiency with chunking or something like that, maybe a distance check or an in-frame/in-window check if possible
/// @brief handle collisions and m_player CState updates; includes tile matrix, CTransform, CState, CBoundingBox, CInput
void ScenePlay::sObjectCollision()
{
    PROFILE_FUNCTION();

    // cache once per frame
    const std::vector<std::vector<Entity>>& tileMatrix = m_entityManager.getTileMatrix();

    playerTileCollisions(tileMatrix);

    /// TODO: weapon-tile collisions (like pistol that fell out of someones hand when killed), other object collisions

    // ragdoll-tile collisions /// TODO: could just do two vertices on a stick and call it a day (or give the vertices a circular distance for collisions)
    for (Entity& rag : m_entityManager.getEntities("ragdoll"))
    {
        CTransform& trans = rag.getComponent<CTransform>();
        CBoundingBox& box = rag.getComponent<CBoundingBox>();

        std::array<Vec2f, 4> vertices;
        float halfDiag = sqrtf(box.size.x * box.size.x + box.size.y * box.size.y) / 2.0f;
        float angleToVertex0 = asinf(box.halfSize.y / halfDiag); // bottom-right (without trans.angle)
        float angleToVertex1 = M_PI - angleToVertex0; // bottom-left (without trans.angle)
        float angleToVertex2 = M_PI + angleToVertex0;
        float angleToVertex3 = 2.0f * M_PI - angleToVertex0;
        vertices[0] = trans.pos + Vec2f(cosf(angleToVertex0 + trans.angle) * halfDiag, sinf(angleToVertex0 + trans.angle) * halfDiag);
        vertices[1] = trans.pos + Vec2f(cosf(angleToVertex1 + trans.angle) * halfDiag, sinf(angleToVertex1 + trans.angle) * halfDiag);
        vertices[2] = trans.pos + Vec2f(cosf(angleToVertex2 + trans.angle) * halfDiag, sinf(angleToVertex2 + trans.angle) * halfDiag);
        vertices[3] = trans.pos + Vec2f(cosf(angleToVertex3 + trans.angle) * halfDiag, sinf(angleToVertex3 + trans.angle) * halfDiag);

        /// TODO: change entirely maybe, if too slow down the line
        std::array<Vec2f, 4> prevVertices;
        angleToVertex0 = asinf(box.halfSize.y / halfDiag);
        angleToVertex1 = M_PI - angleToVertex0;
        angleToVertex2 = M_PI + angleToVertex0;
        angleToVertex3 = 2.0f * M_PI - angleToVertex0;
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

        for (int i = 0; i < 4; ++i)
        {
            Vec2f vert = vertices[i];
            Vec2f prevVert = prevVertices[i];
            Vec2i gridPos = vert.to<int>() / m_cellSizePixels;

            std::cout << "vertex " << i << ": " << vert.x << " " << vert.y << std::endl;

            /// TODO: edge case: vert x = 1300 so grid pos = 130, but no resolutions needs to happen
            /// TODO: maybe check multiple times per frame for more accuracy
            if (tileMatrix[gridPos.x][gridPos.y].isActive()) // vertex inside tile /// TODO: possible seg faults
            {
                // std::cout << "collision with tile" << std::endl;

                float bounce = 0.7f;
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
                        std::cout << "collided from top: " << normalForceMag << " " << frictionForceMag << "\n";
                        if (abs(normalForceMag) >= threshold || abs(frictionForceMag) >= threshold)
                            Physics::ForceEntity(rag, Vec2f(travel.x > 0 ? -frictionForceMag : frictionForceMag, -normalForceMag), vert);
                        else
                        {
                            std::cout << "skipping force" << std::endl;
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
                        std::cout << "collided from bottom: " << normalForceMag << " " << frictionForceMag << "\n";
                        if (abs(normalForceMag) >= threshold || abs(frictionForceMag) >= threshold)
                            Physics::ForceEntity(rag, Vec2f(travel.x > 0 ? -frictionForceMag : frictionForceMag, normalForceMag), vert);
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
                        std::cout << "collided from left: " << normalForceMag << " " << frictionForceMag << "\n";
                        if (abs(normalForceMag) >= threshold || abs(frictionForceMag) >= threshold)
                            Physics::ForceEntity(rag, Vec2f(-normalForceMag, travel.y > 0 ? -frictionForceMag : frictionForceMag), vert); /// TODO: add some sort of if force less than certain amount just make velocity zero so that is doesn't infinitely bounce at tiny bounce amounts
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
                        std::cout << "collided from right: " << normalForceMag << " " << frictionForceMag << "\n";
                        if (abs(normalForceMag) >= threshold || abs(frictionForceMag) >= threshold)
                            Physics::ForceEntity(rag, Vec2f(normalForceMag, travel.y > 0 ? -frictionForceMag : frictionForceMag), vert);
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
                    std::cout << "no previous overlap, skipping\n";
                }
                // }
                /// TODO: no previous overlap?
                /// TODO: trans.pos close enough to trans.prevPos, then don't make a physics update, just freeze it until there is no collision again
            }
        }
    }
}

/// @brief handle all weapon firing logic (and melee if implemented) and projectile movement; decoupled from other entities since updated multiple times per frame; includes CInput, CFireRate, CTransform, CDamage, CHealth, CType, tile matrix
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
    CFireRate& fireRate = m_weapon.getComponent<CFireRate>();
    std::chrono::steady_clock::time_point now = std::chrono::high_resolution_clock::now();
    if (input.shoot && (now - fireRate.lastShotTime).count() >= 1000000000.0f / fireRate.fireRate)
    {
        fireRate.lastShotTime = now;
        spawnBullet(m_weapon);
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
            setPaused(!m_paused);
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

    /// TODO: Complete the Animation class code first
    // for each entity with an animation, call entity->get<CAnimation>().animation.update()
    // if animation is not repeated, and it has ended, destroy the entity

    // set animation of player based on its CState component
}

/// @brief handles camera view logic; includes CTransform
void ScenePlay::sCamera()
{
    PROFILE_FUNCTION();

    Vec2f& pPos = m_player.getComponent<CTransform>().pos;

    // center the view on the player
    /// TODO: deal with the edge of world view, prolly make it so that view center is always at player center for competitive fairness and less disorientation, will need a way to keep players inside world bound tho (force or invisible wall with background or darkness or just more tiles that extend out of sight, could also throw in some easter eggs / secrets there
    const Vec2i viewSize(m_game.window().getSize().x, m_game.window().getSize().y); /// TODO: is this better than just getting an unsigned Vec2 reference (Vec2ui&)? Change back when doing the data type optimizations if I add back Vec2ui and such
    float viewCenterX = std::clamp(pPos.x, viewSize.x / 2.0f, m_worldMaxPixels.x - viewSize.x / 2.0f);
    float viewCenterY = std::clamp(pPos.y, viewSize.y / 2.0f, m_worldMaxPixels.y - viewSize.y / 2.0f);
    m_mainView.setCenter({ viewCenterX, viewCenterY });
    m_miniMapView.setCenter({ viewCenterX, viewCenterY });

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
    const std::vector<std::vector<Entity>>& tileMatrix = m_entityManager.getTileMatrix();


    /// draw all entity textures / animations in layers

    // collidable layer (tiles, player, bullets, items), this comes last so it's always visible
    Vec2i playerGridPos = (playerTrans.pos / m_cellSizePixels).to<int>(); // signed, for operations below /// NOTE: grid pos 0 means pixel 0 through 9

    if (m_drawTextures)

        PROFILE_SCOPE("rendering textures");

    const Vec2f& mainViewSize = m_mainView.getSize(); //  window size is the view size now

    Vec2i checkLength = (mainViewSize / m_cellSizePixels / 2.0f).to<int>();

    // limits on grid coords to check
    int minX = std::max(0, playerGridPos.x - checkLength.x);
    int maxX = std::min(m_worldMaxCells.x - 1, playerGridPos.x + checkLength.x);
    int minY = std::max(0, playerGridPos.y - checkLength.y);
    int maxY = std::min(m_worldMaxCells.y - 1, playerGridPos.y + checkLength.y);

    // find open air tiles method for visible tiles
    std::vector<Vec2i> openTiles; /// TODO: use these for vertices method, might not even need it and could just use visited
    std::stack<Vec2i> tileStack;
    std::vector<std::vector<bool>> visited(mainViewSize.x / m_cellSizePixels + 1, std::vector<bool>(mainViewSize.y / m_cellSizePixels + 1)); // + 1 because including min and max values /// TODO: could add a visited property to each tile instead but this is good for now

    findOpenTiles(playerGridPos.x, playerGridPos.y, minX, maxX, minY, maxY, tileMatrix, openTiles, tileStack, visited);

    // gonna render tiles visited
    m_tileTexture.clear();
    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            if (visited[x - minX][y - minY])
            {
                if (tileMatrix[x][y].isActive())
                {
                    sf::RectangleShape block = sf::RectangleShape({ 1, 1 }); // 1 pixel
                    CColor& color = tileMatrix[x][y].getComponent<CColor>();
                    block.setFillColor(sf::Color(color.r, color.g, color.b));
                    block.setPosition({ static_cast<float>(x - minX), static_cast<float>(y - minY) });
                    m_tileTexture.draw(block);

                    /// TODO: draw neighbors with less lighting
                    // if (x >= minX && tileMatrix[x - 1][y].isActive() && tileMatrix[x - 1][y].light < 1.0f)
                    // {
                    //     color = tileMatrix[x - 1][y].getComponent<CColor>();
                    //     block.setFillColor(sf::Color(color.r, color.g, color.b, 0.5f));
                    //     block.setPosition({ static_cast<float>((x - 1) * m_cellSizePixels.x), static_cast<float>(y * m_cellSizePixels.y) });
                    //     window.draw(block);
                    //     if (check this one's neighbors for light < 0.5f)
                    //     {

                    //     }
                    // }
                    // if (x <= maxX && tileMatrix[x + 1][y].isActive() && tileMatrix[x - 1][y].light < 1.0f)
                    // {

                    // }
                    // if (y >= minY && tileMatrix[x][y - 1].isActive() && tileMatrix[x - 1][y].light < 1.0f)
                    // {

                    // }
                    // if (y <= maxY && tileMatrix[x][y + 1].isActive() && tileMatrix[x - 1][y].light < 1.0f)
                    // {

                    // }
                }
                /// TODO: render visited backgrounds and decoration and such differently if they arent 10x10 sized pixels only (static background blocks can be I suppose, but not the main parallax background)
                // else
                // {
                //     sf::RectangleShape block = sf::RectangleShape({ static_cast<float>(m_cellSizePixels), static_cast<float>(m_cellSizePixels) });
                //     block.setFillColor(sf::Color(100, 100, 100));
                //     block.setPosition({ static_cast<float>(x * m_cellSizePixels), static_cast<float>(y * m_cellSizePixels) });
                //     window.draw(block);
                // }
            }
        }
    }
    m_tileTexture.display();
    sf::Sprite scaledTiles(m_tileTexture.getTexture()); /// TODO: consider another approach if sprite creation is long
    scaledTiles.setScale({ static_cast<float>(m_cellSizePixels), static_cast<float>(m_cellSizePixels) });
    scaledTiles.setPosition({ static_cast<float>(minX * m_cellSizePixels), static_cast<float>(minY * m_cellSizePixels) });
    window.draw(scaledTiles);

    // players
    if (m_player.isActive()) /// TODO: maybe change this to for all players or something
    {
        // player 
        sf::Sprite& playerSprite = m_player.getComponent<CAnimation>().animation.getSprite();
        playerSprite.setPosition(playerTrans.pos);
        playerSprite.setScale(playerTrans.scale);
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
    }

    // ragdolls
    for (Entity& rag : m_entityManager.getEntities("ragdoll"))
    {
        const CTransform& trans = rag.getComponent<CTransform>();
        sf::Sprite& sprite = rag.getComponent<CAnimation>().animation.getSprite();
        sprite.setPosition(trans.pos);
        sprite.setRotation(sf::radians(trans.angle));
        window.draw(sprite);
    }

    // weapons
    // const CTransform& weaponTrans = m_weapon.getComponent<CTransform>();
    // sf::Sprite& weaponSprite = m_weapon.getComponent<CAnimation>().animation.getSprite();
    // weaponSprite.setPosition(weaponTrans.pos);
    // weaponSprite.setScale(weaponTrans.scale);
    // weaponSprite.setRotation(sf::radians(weaponTrans.angle));
    // window.draw(weaponSprite);

    // bullets
    for (Entity& bullet : m_entityManager.getEntities("bullet"))
    {
        const CTransform& transform = bullet.getComponent<CTransform>();

        sf::Sprite& sprite = bullet.getComponent<CAnimation>().animation.getSprite();
        sprite.setRotation(sf::radians(transform.angle));
        sprite.setPosition(transform.pos);
        sprite.setScale(transform.scale);

        window.draw(sprite);
    }

    // use open-air tiles with ray casting 
    std::vector<Vec2f> vertices; // in pixels
    std::unordered_set<Vec2f> vertexSet; // use unordered set to collect unique vertices and put only unique ones in vector

    // four corners of the screen
    /// TODO: see if the + 1 is necessary now that they're floats
    vertices.emplace_back(window.getView().getCenter().x - mainViewSize.x / 2.0f, window.getView().getCenter().y - mainViewSize.y / 2.0f);
    vertices.emplace_back(window.getView().getCenter().x + mainViewSize.x / 2.0f, window.getView().getCenter().y - mainViewSize.y / 2.0f);
    vertices.emplace_back(window.getView().getCenter().x - mainViewSize.x / 2.0f, window.getView().getCenter().y + mainViewSize.y / 2.0f); /// TODO: these may not be necessary of players will never reach bottom of world
    vertices.emplace_back(window.getView().getCenter().x + mainViewSize.x / 2.0f, window.getView().getCenter().y + mainViewSize.y / 2.0f); /// TODO: these may not be necessary of players will never reach bottom of world

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
    const Vec2f playerPos = playerTrans.pos;
    std::vector<Vec2f> verticesToAdd;
    for (int i = 0; i < vertices.size(); ++i)
    {
        Vec2f& vertex = vertices[i];
        float rayAngle = vertex.angleFrom(playerPos);

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
        int xCoord = playerGridPos.x;
        int yCoord = playerGridPos.y;

        // way we step in x or y when traveling the ray
        Vec2i rayStep;

        // accumulated distances (pixels) in the direction of the hypoteneuse caused by a change in x/y from the start of the ray (player position), starting with initial pixel offset from grid coord (top-left)
        float xTravel, yTravel;

        if (rayUnitVec.x < 0)
        {
            rayStep.x = -1;
            xTravel = (playerPos.x - (xCoord * m_cellSizePixels)) * xMoveHypDist;
        }
        else
        {
            rayStep.x = 1;
            xTravel = (m_cellSizePixels - (playerPos.x - (xCoord * m_cellSizePixels))) * xMoveHypDist;
        }

        if (rayUnitVec.y < 0)
        {
            rayStep.y = -1;
            yTravel = (playerPos.y - (yCoord * m_cellSizePixels)) * yMoveHypDist;
        }
        else
        {
            rayStep.y = 1;
            yTravel = (m_cellSizePixels - (playerPos.y - (yCoord * m_cellSizePixels))) * yMoveHypDist;
        }

        bool tileHit = false;
        bool vertexReached = false;
        // bool topLeft, topRight, bottomLeft, bottomRight;
        while (!tileHit && !vertexReached) /// TODO: alternatively, add a xTravel < rayLength - 0.001f or something instead of vertexReached
        {
            if (xTravel < yTravel)
            {
                xCoord += rayStep.x;

                // at this point, the endpoint of the line formed by playerPos + xTravel * rayUnitVec is the collision point on tile (xCoord, yCoord) if tile is active there

                xTravel += xMoveHypDist * m_cellSizePixels;
            }
            else
            {
                yCoord += rayStep.y;
                yTravel += yMoveHypDist * m_cellSizePixels;
            }

            if (tileMatrix[xCoord][yCoord].isActive()) // tile hit /// TODO: seg fault on edges of world or if second check fails and this goes on
            {
                tileHit = true;

                // remove vertex from vertices
                vertex = vertices.back();
                vertices.pop_back();
                --i;
            }

            // if vertex reached and not originally unique (another tile shares it so don't go through) just keep vertex point, do nothing
            /// TODO: keep repeats? given this thought above ^
            if (!tileMatrix[xCoord][yCoord].isActive() && (vertex.x >= xCoord * m_cellSizePixels && vertex.x <= (xCoord + 1) * m_cellSizePixels && vertex.y >= yCoord * m_cellSizePixels && vertex.y <= (yCoord + 1) * m_cellSizePixels)) // vertex reached since in cell with no tile but with this vertex
            {
                vertexReached = true;
            }
        }
        // }

        /// find vertex and divert angle method, less rays, but possibly still more work
        // note that the top two corners of the screen are reached as well as actual vertices
        if (vertexReached)
        {
            /// if vertex reached and if just passed there is no tile, expand line to next intersection or end of screen and create new point there for triangle fan

            // check for tile just passed the vertex in the direction of the ray
            Vec2i checkCoord = ((vertex.to<float>() + rayUnitVec) / m_cellSizePixels).to<int>();

            if (!(checkCoord.x < minX || checkCoord.x > maxX || checkCoord.y < minY || checkCoord.y > maxY || tileMatrix[checkCoord.x][checkCoord.y].isActive()))
            {
                /// add small angle to ray 
                /// NOTE: angle starts at 0 on +x-axis and increases in a CW manner up to 2π (since +y-axis points down)
                /// TODO: is this more expensive than just adding two more rays at slight angle offsets for each ray?
                /// TODO: handle pure vertical and horizontal rays case

                float dTheta = 0.0001f;
                float newAngle;
                if (rayUnitVec.x < 0 && rayUnitVec.y < 0) // came from bottom right
                {
                    if (tileMatrix[xCoord - 1][yCoord].isActive() && tileMatrix[xCoord][yCoord - 1].isActive()) // shared vertex
                        continue; // don't want ray shining through diagonal lines of tiles

                    if (tileMatrix[xCoord - 1][yCoord].isActive()) // tile to the left active
                        newAngle = rayAngle + dTheta; // CW
                    else // tile above active
                        newAngle = rayAngle - dTheta; // CCW
                }
                else if (rayUnitVec.x > 0 && rayUnitVec.y < 0) // came from bottom left
                {
                    if (tileMatrix[xCoord][yCoord - 1].isActive() && tileMatrix[xCoord + 1][yCoord].isActive())
                        continue;

                    if (tileMatrix[xCoord][yCoord - 1].isActive()) // tile above active
                        newAngle = rayAngle + dTheta; // CW
                    else // tile to the right active
                        newAngle = rayAngle - dTheta;; // CCW
                }
                else if (rayUnitVec.x > 0 && rayUnitVec.y > 0) // came from top left
                {
                    if (tileMatrix[xCoord + 1][yCoord].isActive() && tileMatrix[xCoord][yCoord + 1].isActive())
                        continue;

                    if (tileMatrix[xCoord + 1][yCoord].isActive()) // tile to the right
                        newAngle = rayAngle + dTheta; // CW
                    else // tile below is active
                        newAngle = rayAngle - dTheta; // CCW
                }
                else // came from top right or /// TODO: a horizontal/vertical ray case
                {
                    if (tileMatrix[xCoord][yCoord + 1].isActive() && tileMatrix[xCoord - 1][yCoord].isActive())
                        continue;

                    if (tileMatrix[xCoord][yCoord + 1].isActive()) // tile below is active
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
                xCoord = checkCoord.x;
                yCoord = checkCoord.y;

                while (!tileHit)
                {
                    if (xTravelNew < yTravelNew)
                    {
                        xCoord += rayStep.x;

                        // at this point, the endpoint of the line formed by vertex + xTravelNew * newRayUnitVec is the collision point on tile (xCoord, yCoord) if tile is active there

                        if (xCoord < minX || xCoord > maxX || tileMatrix[xCoord][yCoord].isActive()) /// TODO: use ==? 
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
                        yCoord += rayStep.y;

                        if (yCoord < minY || yCoord > maxY || tileMatrix[xCoord][yCoord].isActive()) /// TODO: use ==?
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
    std::sort(vertices.begin(), vertices.end(), [&playerPos](const Vec2f& a, const Vec2f& b) { return a.angleFrom(playerPos) < b.angleFrom(playerPos); });

    // create triangle fan of vertices (pixels)
    std::vector<Vec2f> triangleFan;
    triangleFan.push_back(playerPos);
    triangleFan.insert(triangleFan.end(), vertices.begin(), vertices.end());
    triangleFan.push_back(vertices.front());

    // render things inside triangle fan
    sf::VertexArray fan(sf::PrimitiveType::TriangleFan, triangleFan.size());
    for (int i = 0; i < triangleFan.size(); ++i)
    {
        fan[i].position = sf::Vector2f(triangleFan[i].x, triangleFan[i].y);
        fan[i].color = sf::Color(255, 255, 255, 100);

        // sf::CircleShape dot(2);
        // dot.setPosition({ static_cast<float>(triangleFan[i].x - 2), static_cast<float>(triangleFan[i].y - 2) });
        // dot.setFillColor(sf::Color(0, 0, 255, 100));
        // window.draw(dot);
    }
    window.draw(fan);



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


/// draw all entity collision bounding boxes with a rectangle

// if (m_drawCollision)
// {
//     sf::CircleShape dot(4);
//     dot.setFillColor(sf::Color::Black);
//     for (int x = 0; x < m_worldMaxCells.x; ++x)
//     {
//         for (int y = 0; y < m_worldMaxCells.y; ++y)
//         {
//             const Entity& tile = tileMatrix[x][y];
//             CBoundingBox& box = tile.getComponent<CBoundingBox>();
//             CTransform& transform = tile.getComponent<CTransform>();
//             sf::RectangleShape rect;
//             rect.setSize(Vec2f(box.size.x - 1, box.size.y - 1)); // - 1 cuz line thickness of 1?
//             rect.setOrigin(box.halfSize);
//             rect.setPosition(transform.pos);
//             rect.setFillColor(sf::Color(0, 0, 0, 0));
//             rect.setOutlineColor(sf::Color(255, 255, 255, 255));
//             // if (box.blockMove && box.blockVision)
//             // {
//             //     rect.setOutlineColor(sf::Color::Black);
//             // }
//             // if (box.blockMove && !box.blockVision)
//             // {
//             //     rect.setOutlineColor(sf::Color::Blue);
//             // }
//             // if (!box.blockMove && box.blockVision)
//             // {
//             //     rect.setOutlineColor(sf::Color::Red);
//             // }
//             // if (!box.blockMove && !box.blockVision)
//             // {
//             //     rect.setOutlineColor(sf::Color::White);
//             // }
//             rect.setOutlineThickness(1);
//             window.draw(rect);
//         }
//         // if (e.hasComponent<CPatrol>())
//         // {
//         //     auto& patrol = e.getComponent<CPatrol>().positions;
//         //     for (int p = 0; p < patrol.size(); p++)
//         //     {
//         //         dot.setPosition(patrol[p]);
//         //         window.draw(dot);
//         //     }
//         // }
//         // if (e.hasComponent<CFollowPlayer>())
//         // {
//         //     sf::VertexArray lines(sf::PrimitiveType::LineStrip, 2);
//         //     lines[0].position.x = e.getComponent<CTransform>().pos.x;
//         //     lines[0].position.y = e.getComponent<CTransform>().pos.y;
//         //     lines[0].color = sf::Color::Black;
//         //     lines[1].position.x = m_player.getComponent<CTransform>().pos.x;
//         //     lines[1].position.y = m_player.getComponent<CTransform>().pos.y;
//         //     lines[1].color = sf::Color::Black;
//         //     window.draw(lines);
//         //     dot.setPosition(e.getComponent<CFollowPlayer>().home);
//         //     window.draw(dot);
//         // }
//     }
// }

/// fps counter

    float elapsedTime = m_fpsClock.restart().asSeconds();
    float fps = 1.0f / elapsedTime;
    m_fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));

    // draw the fps text on the default view (w.r.t. window coordinates, not game world)
    window.setView(window.getDefaultView());
    window.draw(m_fpsText);

    /// TODO: line of sight without cone? triangle fan method, ray casting
    // ideas:
        // 1. player's current (x, y), then do some sort of search or something to find all end nodes where tile is active and check only those
        // 1.1. cast rays to vertices of those tiles only (can manage) and do triangle fan method

        // 3. turn tile map into polygon with only outside edges (has other physics advantages as well for collisions and such and can have edges that don't align with axes and so on), only regen poly map if tile map changes, then use vertices from poly map with 3 ray per vertex method or my angle offset method implemented now, could do line segment intersections (implemented in Physics) or incremental tile checks like implemented now
        // 3.1. https://www.youtube.com/watch?v=fc3nnG2CG8U&t=17s&ab_channel=javidx9

    // TODO: lighting of tiles under others
    // somehow incorporate the fact that (maybe not here but somewhere) I want to see more than just one layer of tiles deep
        // idea: render all tiles who have a vertex included in the triangle fan
        // then propagate 50% light to the neighbors of those tiles if light < 100% (so we don't do it to those tiles), then to neighbors neighbors if light < 50% (again, the if's make sure we arent checking already checked tiles), etc.

    /// TODO: light cone: see in direction of pointer only
    // get mouse pointer pos
    // create 2 vectors at angles -x and x away from player-to-pointer vector
    // line segment from player to edge of screen and check for intersection, first intersection visible, rest not
    // use a shader?
    // triangle fan with limited angle
    // entity can see up to closest intersection point
    // lighting effects : light travels outward in all directions
    // cast rays at all vertices, shine two extras for each ray just to the left and right of it to have rays that go all the way to the edges of the screen, then connect all endpoints of lines and fill in the light
    // can shine a few more rays to get a shadow look
    // may be able to do this my own way since places where light should extend(but wouldn't with the connect-the-dots method if not using the extra two ways for every vertex-aiming ray) do not have a ray that intersects a line between them

    /// minimap 

    // if (m_drawMinimap)
    // {
    //     PROFILE_SCOPE("rendering minimap");

    //     window.setView(m_miniMapView);
    //     const Vec2f& viewSize = m_miniMapView.getSize();

    //     // background
    //     sf::RectangleShape minimapBackground(viewSize);
    //     minimapBackground.setPosition({ m_miniMapView.getCenter().x - viewSize.x / 2.0f, m_miniMapView.getCenter().y - viewSize.y / 2.0f });
    //     minimapBackground.setFillColor(sf::Color(50, 50, 50));
    //     window.draw(minimapBackground);

    //     // player icon
    //     sf::CircleShape player(10);
    //     player.setFillColor(sf::Color::Green);
    //     player.setPosition({ m_miniMapView.getCenter().x - 5, m_miniMapView.getCenter().y - 5 });
    //     window.draw(player);

    //     // the rest
    //     int horizontalCheckLength = static_cast<int>(viewSize.x / m_cellSizePixels.x / 2.0f);
    //     int verticalCheckLength = static_cast<int>(viewSize.y / m_cellSizePixels.y / 2.0f);

    //     int minX = std::max(0, playerGridPosX - horizontalCheckLength);
    //     int maxX = std::min(static_cast<int>(m_worldMaxCells.x) - 1, playerGridPosX + horizontalCheckLength);
    //     int minY = std::max(0, playerGridPosY - verticalCheckLength);
    //     int maxY = std::min(static_cast<int>(m_worldMaxCells.y) - 1, playerGridPosY + verticalCheckLength);

    //     // float scaleFactor = 0.2f;
    //     // sf::RectangleShape tileRect(sf::Vector2f(m_cellSizePixels.x, m_cellSizePixels.y));
    //     // sf::Color tileColor;

    //     for (int x = minX; x <= maxX; ++x)
    //     {
    //         for (int y = minY; y <= maxY; ++y)
    //         {
    //             const Entity& tile = tileMatrix[x][y];

    //             if (tile.isActive())
    //             {
    //                 const CTransform& trans = tile.getComponent<CTransform>();

    //                 sf::Sprite& sprite = tile.getComponent<CAnimation>().animation.getSprite();

    //                 // sprite.setRotation(sf::radians(trans.rotAngle)); /// TODO: may not even need this either, but may want it for better physics
    //                 sprite.setPosition(trans.pos);
    //                 // sprite.setScale(trans.scale); /// TODO: will I even use scale?

    //                 window.draw(sprite);



    //                 // const std::string& tileType = tile.getComponent<CTile>().type;
    //                 // if (tileType == "dirt") tileColor = sf::Color(139, 69, 19);
    //                 // else if (tileType == "stone") tileColor = sf::Color(128, 128, 128);
    //                 // else if (tileType == "water") tileColor = sf::Color(0, 0, 255);
    //                 // tileColor = sf::Color(139, 69, 19);
    //                 // tileRect.setFillColor(tileColor);
    //                 // tileRect.setPosition(trans.pos);

    //                 // window.draw(tileRect);
    //             }
    //         }
    //     }
    // }

    window.display();
    window.setView(m_mainView);
}

/**
 * helper functions
 */

 /// @brief returns the midpoint of entity based on a given grid position
 /// TODO: eliminating this and just using the top-left positioning like SFML would probably save me seom computation time
Vec2f ScenePlay::gridToMidPixel(const float gridX, const float gridY, const Entity entity)
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

    /// TODO: make sure players and weapons are deleted before new ones are created
    // set player components
    m_player = m_entityManager.addEntity("player");
    m_player.addComponent<CBoundingBox>(Vec2i(m_playerConfig.CW, m_playerConfig.CH), true, true);
    m_player.addComponent<CTransform>(gridToMidPixel(m_worldMaxCells.x / 2, m_worldMaxCells.y / 8, m_player)); // must be after bounding box /// TODO: make spawning in dynamic
    m_player.addComponent<CAnimation>(m_game.assets().getAnimation("woodTall"), false);
    m_player.addComponent<CState>("air");
    m_player.addComponent<CInput>();
    m_player.addComponent<CGravity>(m_playerConfig.GRAVITY);
    m_player.addComponent<CInvincibility>(30); // in frames for now, will change /// TODO: that
    m_player.addComponent<CHealth>(100);

    // spawn player weapon
    m_weapon = m_entityManager.addEntity("weapon");
    m_weapon.addComponent<CFireRate>(12);
    m_weapon.addComponent<CDamage>(50);
    m_weapon.addComponent<CTransform>(m_player.getComponent<CTransform>().pos); /// TODO: make this a lil infront of player
    m_weapon.addComponent<CBoundingBox>(Vec2i(40, 10), false, false); /// TODO: make this dynamic for each weapon
    m_weapon.addComponent<CAnimation>(m_game.assets().getAnimation("Weapon"), false);
    /// TODO: add animation, gravity, bounding box, transform, state, etc. since weapons will drop from player on death
}

/// @brief spawn a bullet at the location of entity traveling toward cursor
void ScenePlay::spawnBullet(Entity entity)
{
    PROFILE_FUNCTION();

    CTransform& entityTrans = entity.getComponent<CTransform>();
    CBoundingBox& entityBox = entity.getComponent<CBoundingBox>();
    Vec2f spawnPos = entityTrans.pos + Vec2f(cosf(entityTrans.angle), sinf(entityTrans.angle)) * entityBox.halfSize.x;
    float bulletSpeed = 1.5f; // number of pixels added to bullet on each update

    const Vec2f& worldTarget = m_game.window().mapPixelToCoords(sf::Mouse::getPosition(m_game.window()));
    const Vec2f bulletVec = worldTarget - entityTrans.pos;

    Entity bullet = m_entityManager.addEntity("bullet");
    bullet.addComponent<CTransform>(spawnPos, bulletVec * bulletSpeed / worldTarget.dist(entityTrans.pos), Vec2f(2.0f, 2.0f), entityTrans.angle, 0.0f);
    bullet.addComponent<CAnimation>(m_game.assets().getAnimation(m_playerConfig.BA), false);
    bullet.addComponent<CLifespan>(300, m_currentFrame);
    bullet.addComponent<CDamage>(entity.getComponent<CDamage>().damage);

    m_game.assets().playSound("Bullet");
}

/// @brief handle player-tile collisions and player state updates
void ScenePlay::playerTileCollisions(const std::vector<std::vector<Entity>>& tileMatrix)
{
    PROFILE_FUNCTION();

    CTransform& playerTrans = m_player.getComponent<CTransform>();
    CBoundingBox& playerBounds = m_player.getComponent<CBoundingBox>();
    std::string& playerState = m_player.getComponent<CState>().state;
    CInput& playerInput = m_player.getComponent<CInput>();

    bool collision = false;

    Vec2i playerGridPos = (playerTrans.pos / m_cellSizePixels).to<int>(); // must be signed as subtraction below 0 is happening
    Vec2i checkLength(playerBounds.halfSize.x / m_cellSizePixels + 1, playerBounds.halfSize.y / m_cellSizePixels + 1);

    int minX = std::max(0, playerGridPos.x - checkLength.x);
    int maxX = std::min(m_worldMaxCells.x - 1, playerGridPos.x + checkLength.x);
    int minY = std::max(0, playerGridPos.y - checkLength.y);
    int maxY = std::min(m_worldMaxCells.y - 1, playerGridPos.y + checkLength.y);

    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            if (tileMatrix[x][y].isActive()) /// TODO: accessing memory in tileMatrix and then switching to entity memory pool, might want local var in tileMatrix or somethin so we don't have to do this
            {
                // finding overlap (without tile bounding boxes)
                float xDiff = abs(playerTrans.pos.x - (x + 0.5f) * m_cellSizePixels);
                float yDiff = abs(playerTrans.pos.y - (y + 0.5f) * m_cellSizePixels);
                float xOverlap = playerBounds.halfSize.x + m_cellSizePixels * 0.5f - xDiff;
                float yOverlap = playerBounds.halfSize.y + m_cellSizePixels * 0.5f - yDiff;

                // there is a collision
                if (xOverlap > 0 && yOverlap > 0)
                {
                    collision = true;

                    // finding previous overlap (without tile bounding boxes)
                    float xPrevDiff = abs(playerTrans.prevPos.x - (x + 0.5f) * m_cellSizePixels);
                    float yPrevDiff = abs(playerTrans.prevPos.y - (y + 0.5f) * m_cellSizePixels);
                    float xPrevOverlap = playerBounds.halfSize.x + m_cellSizePixels * 0.5f - xPrevDiff;
                    float yPrevOverlap = playerBounds.halfSize.y + m_cellSizePixels * 0.5f - yPrevDiff;

                    // we are colliding in y-direction this frame since previous frame already had x-direction overlap
                    if (xPrevOverlap > 0)
                    {
                        // player moving down
                        if (playerTrans.velocity.y > 0)
                        {
                            playerTrans.pos.y -= yOverlap; // player can't fall below tile
                            playerState = abs(playerTrans.velocity.x) > 0 ? "run" : "stand";

                            // jumping
                            if (!m_player.getComponent<CInput>().up) // wait for player to release w key before allowing jump
                            {
                                m_player.getComponent<CInput>().canJump = true; // set to false after jumping in sMovement()
                            }
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

    if (!collision)
    {
        playerState = "air";
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
        playerInput.canJump = true;
    }
}

/// @brief handle bullet-tile collisions
/// TODO: for super fast bullets or just laser tracing whatver it's called, do a line intersect check - easy with tiles since I can just check grid positions along the line from entity to click spot and stop at first intersect
/// TODO: for fast bullets but still projectiles, update the bullet system more than once per game frame
void ScenePlay::projectileTileCollisions(std::vector<std::vector<Entity>>& tileMatrix, std::vector<Entity>& bullets)
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

        if (bulletGridPos.x < 0 || bulletGridPos.x >= m_worldMaxCells.x || bulletGridPos.y < 0 || bulletGridPos.y >= m_worldMaxCells.y)
        {
            continue;
        }

        const Entity& tile = tileMatrix[bulletGridPos.x][bulletGridPos.y];

        if (tile.isActive()) /// TODO: accessing memory in tileMatrix and then switching to entity memory pool, might want local var in tileMatrix or somethin so we don't have to do this, batch operation, if not store tile components (some or all) in the tile matrix
        {
            int& bDamage = bullet.getComponent<CDamage>().damage;
            int& tHealth = tile.getComponent<CHealth>().current;

            tHealth -= bDamage;
            bDamage /= 2;

            if (bDamage <= 0)
            {
                bullet.destroy();
            }
            else if (tile.getComponent<CType>().type == STONE)
            {
                float ricochetChance = generateRandomFloat(0.0f, 1.0f);
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

            /// TODO: could create a toDestory vector and destroy entities all at once somewhere else if faster, test, since we are accessing tileMatrix and then memory pool back and forth for all bullets
            if (tHealth <= 0)
            {
                tile.destroy();

                /// TODO: check neighbors for floating tiles, then apply physics to them (if no close background) by adding a component to them
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
            if (playerInvincibilityTime <= 0 && Physics::IsInside(bullet.getComponent<CTransform>().pos, player))
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
                    spawnRagdoll(player, bullet);

                    player.destroy();
                }
            }
        }
    }
}

void ScenePlay::spawnRagdoll(Entity& player, Entity& bullet)
{
    const CTransform& playerTrans = player.getComponent<CTransform>();
    const CBoundingBox& playerBox = player.getComponent<CBoundingBox>();
    const CTransform& bulletTrans = bullet.getComponent<CTransform>();

    Entity ragdoll = m_entityManager.addEntity("ragdoll");
    ragdoll.addComponent<CTransform>(playerTrans.pos);
    ragdoll.addComponent<CGravity>(m_playerConfig.GRAVITY);
    ragdoll.addComponent<CBoundingBox>(playerBox.size);
    ragdoll.addComponent<CAnimation>(m_game.assets().getAnimation("woodTall"), false);

    Physics::ForceEntity(ragdoll, bulletTrans.velocity * 5.0f, bulletTrans.pos); // arbitrary choice of applied force
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
    std::vector<std::vector<Entity>>& tileMatrix = m_entityManager.getTileMatrix();
    projectileTileCollisions(tileMatrix, projectiles);
    projectilePlayerCollisions(players, projectiles);
}

float ScenePlay::generateRandomFloat(float min, float max)
{
    static std::random_device rd; // Seed
    static std::mt19937 gen(rd()); // Mersenne Twister RNG
    std::uniform_real_distribution<float> dis(min, max); // Range [min, max]

    return dis(gen);
}

/// @brief find tile grid coords that are reachable from (x, y) grid coords without breaking other tiles and add them to openTiles
void ScenePlay::findOpenTiles(int x, int y, const int minX, const int maxX, const int minY, const int maxY, const std::vector<std::vector<Entity>>& tileMatrix, std::vector<Vec2i>& openTiles, std::stack<Vec2i>& tileStack, std::vector<std::vector<bool>>& visited)
{
    // base case - off game world or rendering screen
    /// TODO: seg fault on world edge case, just make world bounds so that camera always in middle and player never reaches "edge"
    if (x < minX || y < minY || x > maxX || y > maxY)
    {
        return;
    }

    // base case - active tile found
    if (tileMatrix[x][y].isActive())
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
        Vec2i topVal = tileStack.top();
        tileStack.pop();
        findOpenTiles(topVal.x, topVal.y, minX, maxX, minY, maxY, tileMatrix, openTiles, tileStack, visited);
    }
}

