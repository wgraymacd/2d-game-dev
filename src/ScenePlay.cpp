/// TODO: multithreading
/// TODO: worry about signed vs unsigned and size and even size_t (adapts to platform's word size) and all that later after learning more about performance differences and such (e.g., may not want to mix signed and unsigned ints like uint32_t and int8_t)
/// TODO: in some cases processing a 64-bit int is faster than a 32-bit one (or 32 faster than 16), but in many cases memory is what slows down a program, so just have to test between memory efficiency and CPU efficiency

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
    registerAction(static_cast<int>(sf::Keyboard::Key::G), "TOGGLE_GRID");
    registerAction(static_cast<int>(sf::Keyboard::Key::M), "TOGGLE_MAP");

    // player keyboard setup
    registerAction(static_cast<int>(sf::Keyboard::Key::W), "JUMP");
    registerAction(static_cast<int>(sf::Keyboard::Key::A), "LEFT");
    registerAction(static_cast<int>(sf::Keyboard::Key::D), "RIGHT");

    // player mouse setup
    registerAction(static_cast<int>(sf::Mouse::Button::Left), "SHOOT", true);

    // grid text setup
    m_gridText.setCharacterSize(12);

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

            // std::cout << "added Tile: " << tile->id() << " " << tile->get<CAnimation>().animation.getName() << " " << tile->get<CTransform>().pos.x << ", " << tile->get<CTransform>().pos.y << std::endl;
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
        - illumiate everything, pretty it up
        - use optimization so things aren't insanely slow
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
                std::cout << "adding tile " << x << ", " << y << std::endl;

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
            sLifespan(); // lifespan calculation first to not waste calculations on dead entities
            sObjectMovement(); // object movement
            sObjectCollision(); // then object collisions
            sProjectiles(); // then iterations of projectile movement and collisions, then projectile spawns
            sAI();
            sAnimation(); // draw all animations (could move this around)
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

 /// @brief handle player, weapon, etc. movement per frame before bullet movement/collision (object = non-projectile)
void ScenePlay::sObjectMovement()
{
    PROFILE_FUNCTION();

    /// player

    std::string& playerState = m_player.getComponent<CState>().state;
    CInput& playerInput = m_player.getComponent<CInput>();
    CTransform& playerTrans = m_player.getComponent<CTransform>();

    Vec2f velToAdd(0.0f, 0.0f);

    /// TODO: consider turing this into real physics
    // gravity
    float airResistance = 15.0f;
    if (playerTrans.velocity.y + m_playerConfig.GRAVITY >= airResistance)
    {
        velToAdd.y += airResistance - playerTrans.velocity.y;
    }
    else
    {
        velToAdd.y += m_playerConfig.GRAVITY;
    }

    // no left or right input - slow down in x-direction (less if in air, more if on ground) until stopped
    if (!playerInput.left && !playerInput.right)
    {
        // float friction = m_playerConfig.SX / 2.0f;
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

        playerTrans.scale.x = abs(playerTrans.scale.x);
        /// TODO: overrite if shooting in other direction (here or maybe in spawnBullet or something)
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

        playerTrans.scale.x = -abs(playerTrans.scale.x);
        /// TODO: overrite if shooting in other direction (here or maybe in spawnBullet or something)
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
    //         std::cout << "playerState set to: " << playerState << std::endl;
    //     }
    //     else
    //     {
    //         playerState = "stand";
    //         std::cout << "playerState set to: " << playerState << std::endl;
    //     }
    // }

    /// weapons
    /// TODO: finish this

    for (Entity& weapon : m_entityManager.getEntities("weapon"))
    {
        weapon.getComponent<CTransform>().pos = playerTrans.pos;
    }
}

/// TODO: modularize some of this if needed to reduce repition and make it easier to read
/// TODO: increase efficiency with chunking or something like that, maybe a distance check or an in-frame/in-window check if possible
/// @brief handle collisions and m_player CState updates
void ScenePlay::sObjectCollision()
{
    PROFILE_FUNCTION();

    // cache once per frame
    const std::vector<std::vector<Entity>>& tileMatrix = m_entityManager.getTileMatrix();

    playerTileCollisions(tileMatrix);

    /// TODO: weapon-tile collisions (like pistol that fell out of someones hand when killed), other object collisions
}

/// @brief handle all weapon firing logic (and melee if implemented) and projectile movement
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
/// @brief sets CInput variables according to action, no action logic here
void ScenePlay::sDoAction(const Action& action)
{
    PROFILE_FUNCTION();

    if (action.type() == "START")
    {
        if (action.name() == "TOGGLE_TEXTURE")
        {
            m_drawTextures = !m_drawTextures;
        }
        else if (action.name() == "TOGGLE_COLLISION")
        {
            m_drawCollision = !m_drawCollision;
        }
        else if (action.name() == "TOGGLE_GRID")
        {
            m_drawGrid = !m_drawGrid;
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
    else if (action.type() == "END")
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
/// @brief updates all entities' lifespan and whatever else status
void ScenePlay::sLifespan()
{
    PROFILE_FUNCTION();

    /// TODO: do same locational thing here as with collision and tileMatrix[x][y]
    /// TODO: may want to separate lifespan and health since shit is stored so that components are cached together, or change the way components and entities are stored

    // bullets have lifespan and health
    for (Entity& e : m_entityManager.getEntities("bullet"))
    {
        int& lifespan = e.getComponent<CLifespan>().lifespan;
        if (lifespan <= 0)
        {
            e.destroy();
        }
        else
        {
            lifespan--;
        }
    }

    /// TODO: player has health

    // old code
    // for (auto& e : m_entityManager.getEntities())
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

/// @brief handles camera view logic
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

/// @brief handles all rendering of textures (animations), grid boxes, collision boxes, and fps counter
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
    int playerGridPosX = playerTrans.pos.x / m_cellSizePixels.x; // signed, for operations below /// NOTE: grid pos 0 means pixel 0 through 9
    int playerGridPosY = playerTrans.pos.y / m_cellSizePixels.y;

    if (m_drawTextures)
    {
        PROFILE_SCOPE("rendering textures");

        const Vec2f& viewSize = m_mainView.getSize(); //  window size is the view size now

        int horizontalCheckLength = viewSize.x / m_cellSizePixels.x / 2.0f;
        int verticalCheckLength = viewSize.y / m_cellSizePixels.y / 2.0f;

        // limits on grid coords to check
        int minX = std::max(0, playerGridPosX - horizontalCheckLength);
        int maxX = std::min(m_worldMaxCells.x - 1, playerGridPosX + horizontalCheckLength);
        int minY = std::max(0, playerGridPosY - verticalCheckLength);
        int maxY = std::min(m_worldMaxCells.y - 1, playerGridPosY + verticalCheckLength);

        // trying method 1 for visible tiles
        std::vector<Vec2i> openTiles; /// TODO: use these for vertices method, might not even need it and could just use visited
        std::stack<Vec2i> tileStack;
        std::vector<std::vector<bool>> visited(viewSize.x / m_cellSizePixels.x + 1, std::vector<bool>(viewSize.y / m_cellSizePixels.y + 1)); /// TODO: could add a visited property to each tile instead but this is good for now

        findOpenTiles(playerGridPosX, playerGridPosY, minX, maxX, minY, maxY, tileMatrix, openTiles, tileStack, visited);

        // gonna render everything we visited and nothing else (background and all that that isn't covered by a tile)
        for (int x = minX; x <= maxX; ++x)
        {
            for (int y = minY; y <= maxY; ++y)
            {
                if (visited[x - minX][y - minY])
                {
                    sf::RectangleShape block = sf::RectangleShape(GlobalSettings::cellSizePixels.to<float>());

                    if (tileMatrix[x][y].isActive())
                    {
                        CColor& color = tileMatrix[x][y].getComponent<CColor>();
                        block.setFillColor(sf::Color(color.r, color.g, color.b));
                    }
                    else
                    {
                        block.setFillColor(sf::Color(100, 100, 100));
                    }

                    block.setPosition({ static_cast<float>(x * m_cellSizePixels.x), static_cast<float>(y * m_cellSizePixels.y) });
                    window.draw(block);
                }
            }
        }

        // player
        sf::Sprite& playerSprite = m_player.getComponent<CAnimation>().animation.getSprite();
        playerSprite.setPosition(playerTrans.pos);
        window.draw(m_player.getComponent<CAnimation>().animation.getSprite());

        // bullets
        std::vector<Entity>& bullets = m_entityManager.getEntities("bullet");
        for (Entity& bullet : bullets)
        {
            CTransform& transform = bullet.getComponent<CTransform>();

            sf::Sprite& sprite = bullet.getComponent<CAnimation>().animation.getSprite();
            sprite.setRotation(sf::radians(transform.rotAngle));
            sprite.setPosition(transform.pos);
            sprite.setScale(transform.scale);

            window.draw(sprite);
        }

        // use open-air tiles with ray casting 
        std::vector<Vec2i> vertices; // in pixels
        std::unordered_set<Vec2i> vertexSet; // use unordered set to collect unique vertices and put only unique ones in vector

        // four corners of the screen
        vertices.emplace_back(static_cast<int>(window.getView().getCenter().x - viewSize.x / 2.0f), static_cast<int>(window.getView().getCenter().y - viewSize.y / 2.0f));
        vertices.emplace_back(static_cast<int>(window.getView().getCenter().x + viewSize.x / 2.0f) + 1, static_cast<int>(window.getView().getCenter().y - viewSize.y / 2.0f));
        vertices.emplace_back(static_cast<int>(window.getView().getCenter().x - viewSize.x / 2.0f), static_cast<int>(window.getView().getCenter().y + viewSize.y / 2.0f) + 1); /// TODO: these may not be necessary of players will never reach bottom of world
        vertices.emplace_back(static_cast<int>(window.getView().getCenter().x + viewSize.x / 2.0f) + 1, static_cast<int>(window.getView().getCenter().y + viewSize.y / 2.0f) + 1); /// TODO: these may not be necessary of players will never reach bottom of world

        for (const Vec2i& tileCoords : openTiles)
        {
            Vec2i corners[4] = { tileCoords,
                               { tileCoords.x, tileCoords.y + 1 },
                               { tileCoords.x + 1, tileCoords.y },
                               { tileCoords.x + 1, tileCoords.y + 1 } };

            for (const Vec2i& v : corners)
            {
                if (vertexSet.insert(v * m_cellSizePixels.x).second) // insert() returns {iterator, bool}, bool is true if inserted /// TODO: if x and y cell sizes differ, this gets fucked
                {
                    vertices.push_back(v * m_cellSizePixels.x); /// TODO: if x and y cell sizes differ, this gets fucked
                }
            }
        }

        const Vec2i playerPos = playerTrans.pos.to<int>(); /// TODO: keep float? too inacurate? reference or not?
        for (int i = 0; i < vertices.size(); ++i)
        {
            Vec2i& vertex = vertices[i];
            Vec2i ray = vertex - playerPos;
            float rayLength = ray.length();
            Vec2f rayUnitVec = ray.to<float>() / rayLength;
            float slope = ray.slope();
            float reciprocalSlope = 1.0f / slope;

            float xMoveHypDist = sqrtf(1 + slope * slope); // in grid coords /// TODO: can maybe make int since cells are 10 pixel side lengths and ties aren't that important to perfectly resolve
            float yMoveHypDist = sqrtf(1 + reciprocalSlope * reciprocalSlope); // in grid coords

            // coordinates of grid cell the traveling ray is in
            /// TODO: test edge cases: if hitting right side of tile, I want this coord to truncate to the left side of the tile (since tiles positioned with top-left), this could be wrong, subtly
            int xCoord = playerGridPosX;
            int yCoord = playerGridPosY;

            // way we step in x or y when traveling the ray
            Vec2i rayStep;

            // accumulated distances (pixels) in the direction of the hypoteneuse caused by a change in x/y from the start of the ray (player position), starting with initial pixel offset from grid coord (top-left)
            float xTravel, yTravel;

            if (ray.x < 0)
            {
                rayStep.x = -1;
                xTravel = (playerPos.x - (xCoord * m_cellSizePixels.x)) * xMoveHypDist;
            }
            else
            {
                rayStep.x = 1;
                xTravel = (m_cellSizePixels.x - (playerPos.x - (xCoord * m_cellSizePixels.x))) * xMoveHypDist;
            }

            if (ray.y < 0)
            {
                rayStep.y = -1;
                yTravel = (playerPos.y - (yCoord * m_cellSizePixels.y)) * yMoveHypDist;
            }
            else
            {
                rayStep.y = 1;
                yTravel = (m_cellSizePixels.y - (playerPos.y - (yCoord * m_cellSizePixels.y))) * yMoveHypDist;
            }

            bool tileHit = false;
            while (!tileHit && (xTravel < rayLength - 0.001f || yTravel < rayLength - 0.001f)) // important to have threshold here /// TODO: alternatively, could create a check for if the line has arrived in cell that has that vertex and there is no tile in it, might be able to have one loop for each ray this way; for now, use second loop below
            {
                if (xTravel < yTravel)
                {
                    xCoord += rayStep.x;
                    xTravel += xMoveHypDist * m_cellSizePixels.x;
                }
                else
                {
                    yCoord += rayStep.y;
                    yTravel += yMoveHypDist * m_cellSizePixels.y;
                }

                if (tileMatrix[xCoord][yCoord].isActive()) /// TODO: may want to implement bounds check or think more about this and edge cases like vertex on side of world
                {
                    tileHit = true;

                    // remove vertex from vertices
                    vertex = vertices.back();
                    vertices.pop_back();
                    --i;
                }
            }

            // if vertex reached and if just passed there is no tile, expand line to next intersection or end of screen and create new point there for triangle fan
            /// TODO: add a small bit of angle away from the vertex and tile it's on so that the sorting for the fan works, or do the 3 ray method if needed (see what's faster)
            // while (!tileHit) // another loop to continue the ray to the next tile or edge of screen in the case that the vertex was visible
            // {
            //     if (xTravel < yTravel)
            //     {
            //         xCoord += rayStep.x;
            //         xTravel += xMoveHypDist * m_cellSizePixels.x;

            //         if (xCoord < minX || xCoord > maxX || tileMatrix[xCoord][yCoord].isActive()) /// TODO: use ==? also, first two conditions always checked first?
            //         {
            //             tileHit = true;
            //             vertices.push_back(playerPos + (rayUnitVec * xTravel).to<int>());
            //         }
            //     }
            //     else
            //     {
            //         yCoord += rayStep.y;
            //         yTravel += yMoveHypDist * m_cellSizePixels.y;

            //         if (yCoord < minY || yCoord > maxY || tileMatrix[xCoord][yCoord].isActive()) /// TODO: use ==?
            //         {
            //             tileHit = true;
            //             vertices.push_back(playerPos + (rayUnitVec * yTravel).to<int>());
            //         }
            //     }
            // }
        }

        // if vertex reached and not originally unique (keep repeats? aka tile right away if line continues), just keep vertex point, do nothing
        // somehow incorporate the fact that (maybe not here but somewhere) I want to see more than just one layer of tiles deep
            // idea: render all tiles who have a vertex included in the triangle fan
            // then propagate 50% light to the neighbors of those tiles if light < 100% (so we don't do it to those tiles), then to neighbors neighbors if light < 50% (again, the if's make sure we arent checking already checked tiles), etc.

        // sort the reachable vertices in CCW order 
        /// TODO: consider storing angle for each vertex in the vector, faster probably
        std::sort(vertices.begin(), vertices.end(), [&playerPos](const Vec2i& a, const Vec2i& b) { return a.angleFrom(playerPos) < b.angleFrom(playerPos); });

        // create triangle fan of vertices (pixels)
        std::vector<Vec2i> triangleFan;
        triangleFan.push_back(playerPos);
        triangleFan.insert(triangleFan.end(), vertices.begin(), vertices.end());
        triangleFan.push_back(vertices.front()); // close the shape

        // std::cout << "START START START START x" << std::endl;
        // for (int i = 0; i < triangleFan.size(); ++i)
        // {
        //     std::cout << triangleFan[i].x << std::endl;
        // }
        // std::cout << "END END END END END END x" << std::endl;

        // std::cout << "START START START START y" << std::endl;
        // for (int i = 0; i < triangleFan.size(); ++i)
        // {
        //     std::cout << triangleFan[i].y << std::endl;
        // }
        // std::cout << "END END END END END END y" << std::endl;

        // render things inside triangle fan
        sf::VertexArray fan(sf::PrimitiveType::TriangleFan, triangleFan.size());
        for (int i = 0; i < triangleFan.size(); ++i)
        {
            fan[i].position = sf::Vector2f(triangleFan[i].x, triangleFan[i].y);
            fan[i].color = sf::Color(255, 255, 0, 100);

            sf::CircleShape dot(2); // Radius of 2 pixels
            dot.setPosition({ static_cast<float>(triangleFan[i].x - 2), static_cast<float>(triangleFan[i].y - 2) }); // Center the dot
            dot.setFillColor(sf::Color::Blue);
            window.draw(dot);
        }
        window.draw(fan); /// TODO: still a lil goofy because of angle sorting (e.g., player center pos at same y as a tile, then both vertices visible so both lines at same angle and yeah, must fix)


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
    }

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
    // cannot see tiles behind others
    // ideas:
        // 1. player's current (x, y), then do some sort of search or something to find all end nodes where tile is active and check only those
        // 1.1. cast rays to vertices of those tiles only (can manage) and do triangle fan method
        // 2. cast a certain number of rays equally spaced out (if all vertices on screen is too much) and walk along them until a tile is hit (not as precise, but could maybe blurr around and get better effect anyway?) (could define the number of rays based on tile size so that there is 1 per tile for tiles at edge of screen)
        // 2.1. even if missing a couple tiles with random rays, I could interpolate (just gen the fan) and draw the full tiles in between and it would likely be right
        // 3. turn tile map into polygon with only outside edges (has other physics advantages as well for collisions and such and can have edges that don't align with axes and so on), only regen poly map if tile map changes, then use vertices from poly map with 3 ray per vertex method, could do line segment intersections (implemented in Physics) or incremental tile checks
        // 3.1. https://www.youtube.com/watch?v=fc3nnG2CG8U&t=17s&ab_channel=javidx9


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

 /// @brief helper function for grid drawing; draws line from p1 to p2 on the screen
void ScenePlay::drawLine(const Vec2f& p1, const Vec2f& p2)
{
    PROFILE_FUNCTION();

    sf::Vertex line[] =
    {
        {p1, sf::Color(255, 255, 255, 50)},
        {p2, sf::Color(255, 255, 255, 50)}
    };

    m_game.window().draw(line, 2, sf::PrimitiveType::Lines);
}

/// @brief returns the midpoint of entity based on a given grid position
/// TODO: eliminating this and just using the top-left positioning like SFML would probably save me seom computation time
Vec2f ScenePlay::gridToMidPixel(const float gridX, const float gridY, const Entity entity)
{
    PROFILE_FUNCTION();

    const Vec2f& bBoxHalfSize = entity.getComponent<CBoundingBox>().halfSize;

    float xPos = gridX * m_cellSizePixels.x + bBoxHalfSize.x;
    float yPos = gridY * m_cellSizePixels.y + bBoxHalfSize.y;

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

    // when respawning in same map, must delete previous player
    std::vector<Entity>& entities = m_entityManager.getEntities("player");
    if (!entities.empty())
    {
        entities.front().destroy();
    }

    // set player components
    m_player = m_entityManager.addEntity("player");
    m_player.addComponent<CAnimation>(m_game.assets().getAnimation("woodTall"), true);
    m_player.addComponent<CTransform>(gridToMidPixel(m_worldMaxCells.x / 2, 10, m_player)); /// TODO: make spawning in dynamic
    m_player.addComponent<CBoundingBox>(Vec2i(m_playerConfig.CW, m_playerConfig.CH));
    m_player.addComponent<CState>("air");
    m_player.addComponent<CInput>();
    m_player.addComponent<CGravity>(m_playerConfig.GRAVITY);

    // spawn player weapons
    m_weapon = m_entityManager.addEntity("weapon");
    m_weapon.addComponent<CFireRate>(12);
    m_weapon.addComponent<CDamage>(100);
    m_weapon.addComponent<CTransform>(m_player.getComponent<CTransform>().pos); /// TODO: make this a lil infront of player
    /// TODO: add animation, gravity, bounding box, transform, state, etc. since weapons will drop from player on death
}

/// @brief spawn a bullet at the location of entity traveling toward cursor
void ScenePlay::spawnBullet(Entity entity)
{
    PROFILE_FUNCTION();

    Vec2f& entityPos = entity.getComponent<CTransform>().pos;
    float bulletSpeed = 1.5f; // number of pixels added to bullet on each update

    const Vec2f& worldTarget = m_game.window().mapPixelToCoords(sf::Mouse::getPosition(m_game.window()));
    const Vec2f bulletVec = worldTarget - entityPos;

    Entity bullet = m_entityManager.addEntity("bullet");
    bullet.addComponent<CTransform>
        (
            entityPos,
            bulletVec * bulletSpeed / worldTarget.dist(entityPos),
            Vec2f(3.0f, 3.0f),
            atanf(bulletVec.y / bulletVec.x)
        );
    bullet.addComponent<CAnimation>(m_game.assets().getAnimation(m_playerConfig.BA), true);
    // bullet.addComponent<CBoundingBox>(bullet.getComponent<CAnimation>().animation.getSize());
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

    // check player-tile collisions within a box of size 4 x 4 grid cells around player
    /// TODO: make this box as small as possible for less calculations
    int playerGridPosX = playerTrans.pos.x / m_cellSizePixels.x; // must be signed as subtraction below 0 is happening
    int playerGridPosY = playerTrans.pos.y / m_cellSizePixels.y;
    int horizontalCheckLength = playerBounds.halfSize.x / m_cellSizePixels.x + 1;
    int verticalCheckLength = playerBounds.halfSize.y / m_cellSizePixels.y + 1;

    int minX = std::max(0, playerGridPosX - horizontalCheckLength);
    int maxX = std::min(m_worldMaxCells.x - 1, playerGridPosX + horizontalCheckLength); // ensure m_worldMaxCells != 0 ever or we get wrap around
    int minY = std::max(0, playerGridPosY - verticalCheckLength);
    int maxY = std::min(m_worldMaxCells.y - 1, playerGridPosY + verticalCheckLength);

    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            if (tileMatrix[x][y].isActive()) /// TODO: accessing memory in tileMatrix and then switching to entity memory pool, might want local var in tileMatrix or somethin so we don't have to do this
            {
                // finding overlap (without tile bounding boxes)
                float xDiff = abs(playerTrans.pos.x - (x + 0.5f) * m_cellSizePixels.x);
                float yDiff = abs(playerTrans.pos.y - (y + 0.5f) * m_cellSizePixels.y);
                float xOverlap = playerBounds.halfSize.x + m_cellSizePixels.x * 0.5f - xDiff;
                float yOverlap = playerBounds.halfSize.y + m_cellSizePixels.y * 0.5f - yDiff;

                // Vec2f overlap = Physics::GetOverlap(m_player, tileMatrix[x][y]);

                // there is a collision
                if (xOverlap > 0 && yOverlap > 0)
                {
                    collision = true;

                    // finding previous overlap (without tile bounding boxes)
                    float xPrevDiff = abs(playerTrans.prevPos.x - (x + 0.5f) * m_cellSizePixels.x);
                    float yPrevDiff = abs(playerTrans.prevPos.y - (y + 0.5f) * m_cellSizePixels.y);
                    float xPrevOverlap = playerBounds.halfSize.x + m_cellSizePixels.x * 0.5f - xPrevDiff;
                    float yPrevOverlap = playerBounds.halfSize.y + m_cellSizePixels.y * 0.5f - yPrevDiff;

                    // Vec2f prevOverlap = Physics::GetPreviousOverlap(m_player, tileMatrix[x][y]);

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
                    if (yPrevOverlap > 0)
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

        int bulletGridPosX = bulletTrans.pos.x / m_cellSizePixels.x;
        int bulletGridPosY = bulletTrans.pos.y / m_cellSizePixels.y;

        /// TODO: consider adding a bounding box check for bullets (or just leave them as one pixel at the tip of the bullet so I never have to check), depends on what I want with bullet variety (would just have to copy whats in player tiles with bullets)
        // int horizontalCheckLength = bulletBounds.halfSize.x / m_cellSizePixels.x + 1;
        // int verticalCheckLenght = bulletBounds.halfSize.y / m_cellSizePixels.y + 1;
        /// TODO: if no bounding box, don't even need any internal loop, can just get bullet pos (pixels), see if tile active at grid coord, then say there's a collision and handle it, no isInside or overlap or anything either (already done below)

        if (bulletGridPosX < 0 || bulletGridPosX >= m_worldMaxCells.x || bulletGridPosY < 0 || bulletGridPosY >= m_worldMaxCells.y)
        {
            continue;
        }

        const Entity& tile = tileMatrix[bulletGridPosX][bulletGridPosY];

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

                    if (bulletTrans.prevPos.x > (bulletGridPosX + 1) * m_cellSizePixels.x || bulletTrans.prevPos.x < bulletGridPosX * m_cellSizePixels.x) // colliding from a side
                    {
                        bulletTrans.velocity.x = -bulletTrans.velocity.x;
                    }
                    else // colliding from the top or bottom
                    {
                        bulletTrans.velocity.y = -bulletTrans.velocity.y;
                    }

                    bulletTrans.rotAngle = -bulletTrans.rotAngle;

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

/// @brief move all projectiles and check for collisions
void ScenePlay::updateProjectiles(std::vector<Entity>& projectiles)
{
    PROFILE_FUNCTION();
    // move existing projectiles
    /// TODO: works for bullets, change when adding more projectile types
    for (Entity& p : projectiles)
    {
        CTransform& pTrans = p.getComponent<CTransform>();
        pTrans.prevPos = pTrans.pos;
        pTrans.pos += pTrans.velocity;
    }

    // move other existing projectiles (like bombs, affected by gravity)
    /// TODO: remember to group these checks so that it's fast, might want to use "projectile" tag in entity manager and use an if (hasComponent(<CGravity>)) or whatever to find the bombs vs bullets vs whatever, all in one loop

    // check for collisions with tiles
    std::vector<std::vector<Entity>>& tileMatrix = m_entityManager.getTileMatrix();
    projectileTileCollisions(tileMatrix, projectiles);

    // check for collisions with other players
    /// TODO: projectile-player collisions
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