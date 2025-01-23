/// TODO: multithreading

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

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <fstream>
#include <chrono>

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
            file >> m_playerConfig.GX >> m_playerConfig.GY >> m_playerConfig.CW >> m_playerConfig.CH >> m_playerConfig.SX >> m_playerConfig.SY >> m_playerConfig.SM >> m_playerConfig.GRAVITY >> m_playerConfig.BA;
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
    const std::vector<std::vector<std::string>>& tileMatrix = gen.getTileMatrix();

    // spawn tiles according to their positions in the grid
    for (int x = 0; x < m_worldMaxCells.x; ++x)
    {
        // PROFILE_SCOPE("adding tiles, row x");

        for (int y = 0; y < m_worldMaxCells.y; ++y)
        {
            if (tileMatrix[x][y] == "") continue;

            Entity tile = m_entityManager.addEntity("tile");
            tile.addComponent<CAnimation>(m_game.assets().getAnimation(tileMatrix[x][y]), true); /// TODO: this is what takes so long to gen world, adding components
            tile.addComponent<CTransform>(gridToMidPixel(x, y, tile));
            tile.addComponent<CBoundingBox>(m_game.assets().getAnimation(tileMatrix[x][y]).getSize());
            tile.addComponent<CHealth>(tileMatrix[x][y] == "dirt" ? 40 : 60);

            m_entityManager.addTileToMatrix(tile);
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
    const Vec2ui& viewSize = m_game.window().getSize();
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

    // color the background darker so you know that the game is paused
    if (!m_paused)
    {
        window.clear(sf::Color(5, 5, 5));
    }
    else
    {
        window.clear(sf::Color(10, 10, 10));
    }

    const CTransform& playerTrans = m_player.getComponent<CTransform>();
    const std::vector<std::vector<Entity>>& tileMatrix = m_entityManager.getTileMatrix();

    /// draw all entity textures / animations in layers

    // collidable layer (tiles, player, bullets, items)
    sf::Sprite& playerSprite = m_player.getComponent<CAnimation>().animation.getSprite();
    playerSprite.setPosition(playerTrans.pos);
    window.draw(m_player.getComponent<CAnimation>().animation.getSprite());

    int playerGridPosX = playerTrans.pos.x / m_cellSizePixels.x;
    int playerGridPosY = playerTrans.pos.y / m_cellSizePixels.y;

    if (m_drawTextures)
    {
        PROFILE_SCOPE("rendering textures");

        const Vec2f& viewSize = m_mainView.getSize(); //  window size is the view size now

        int horizontalCheckLength = static_cast<int>(viewSize.x / m_cellSizePixels.x / 2.0f); /// TODO: change this + 1 if needed, test
        int verticalCheckLength = static_cast<int>(viewSize.y / m_cellSizePixels.y / 2.0f);

        int minX = std::max(0, playerGridPosX - horizontalCheckLength);
        int maxX = std::min(static_cast<int>(m_worldMaxCells.x) - 1, playerGridPosX + horizontalCheckLength);
        int minY = std::max(0, playerGridPosY - verticalCheckLength);
        int maxY = std::min(static_cast<int>(m_worldMaxCells.y) - 1, playerGridPosY + verticalCheckLength);

        for (int x = minX; x <= maxX; ++x)
        {
            for (int y = minY; y <= maxY; ++y)
            {
                if (tileMatrix[x][y].isActive())
                {
                    const Entity& tile = tileMatrix[x][y];

                    CTransform& trans = tile.getComponent<CTransform>();
                    sf::Sprite& sprite = tile.getComponent<CAnimation>().animation.getSprite();
                    // sprite.setRotation(sf::radians(trans.rotAngle)); /// TODO: may not even need this either, but may want it for better physics
                    sprite.setPosition(trans.pos);
                    // sprite.setScale(trans.scale); /// TODO: will I even use scale?

                    window.draw(sprite);
                }
            }
        }

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

        /// grid

        // if (m_drawGrid)
        // {
        //     float leftX = window.getView().getCenter().x - (window.getView().getSize().x / 2);
        //     float rightX = leftX + window.getView().getSize().x; // + m_cellSizePixels.x maybe
        //     // logic if grid (0, 0) at bottom left
        //     // float bottomY = window.getView().getCenter().y + (window.getView().getSize().y / 2);
        //     // float topY = bottomY - window.getView().getSize().y;
        //     // logic if grid (0, 0) at top left
        //     float topY = window.getView().getCenter().y - window.getView().getSize().y / 2;
        //     float bottomY = topY + window.getView().getSize().y; // + m_cellSizePixels.y maybe

        //     float nextGridX = leftX - fmodf(leftX, m_cellSizePixels.x);
        //     // logic if grid (0, 0) at bottom left
        //     // float nextGridY = bottomY - fmodf(bottomY, m_cellSizePixels.y);
        //     // logic if grid (0, 0) at top left
        //     float nextGridY = topY - fmodf(topY, m_cellSizePixels.y);

        //     // vertical grid lines
        //     for (float x = nextGridX; x <= rightX; x += m_cellSizePixels.x)
        //     {
        //         drawLine(Vec2f(x, topY), Vec2f(x, bottomY));
        //     }

        //     // horizontal grid lines
        //     // logic if grid (0, 0) at bottom left
        //     // for (float y = nextGridY; y >= topY; y -= m_cellSizePixels.y)
        //     // logic if grid (0, 0) at top left
        //     for (float y = nextGridY; y <= bottomY; y += m_cellSizePixels.y)
        //     {
        //         drawLine(Vec2f(leftX, y), Vec2f(rightX, y));

        //         // grid cell labels
        //         for (float x = nextGridX; x <= rightX; x += m_cellSizePixels.x)
        //         {
        //             std::string xCell = std::to_string(static_cast<int>(x / m_cellSizePixels.x));
        //             // logic if grid (0, 0) at bottom left
        //             // std::string yCell = std::to_string(static_cast<int>((bottomY - y) / m_cellSizePixels.y));
        //             // logic if grid (0, 0) at top left
        //             std::string yCell = std::to_string(static_cast<int>(y / m_cellSizePixels.y));

        //             m_gridText.setString("(" + xCell + "," + yCell + ")");
        //             // m_gridText.setPosition(x + 3, y - m_cellSizePixels.y + 2); // position label inside cell, bottom left (0, 0)
        //             m_gridText.setPosition({ x + 3, y + 2 }); // position label inside cell, top left (0, 0)
        //             m_gridText.setFillColor(sf::Color(255, 255, 255, 50));
        //             window.draw(m_gridText);
        //         }
        //     }
        // }
    }

    /// draw all entity collision bounding boxes with a rectangle

    // if (m_drawCollision)
    // {
    //     sf::CircleShape dot(4);
    //     dot.setFillColor(sf::Color::Black);
    //     for (auto e : m_entityManager.getEntities())
    //     {
    //         if (e.hasComponent<CBoundingBox>())
    //         {
    //             auto& box = e.getComponent<CBoundingBox>();
    //             auto& transform = e.getComponent<CTransform>();
    //             sf::RectangleShape rect;
    //             rect.setSize(Vec2f(box.size.x - 1, box.size.y - 1)); // - 1 cuz line thickness of 1?
    //             rect.setOrigin(box.halfSize);
    //             rect.setPosition(transform.pos);
    //             rect.setFillColor(sf::Color(0, 0, 0, 0));
    //             rect.setOutlineColor(sf::Color(255, 255, 255, 255));

    //             if (box.blockMove && box.blockVision)
    //             {
    //                 rect.setOutlineColor(sf::Color::Black);
    //             }
    //             if (box.blockMove && !box.blockVision)
    //             {
    //                 rect.setOutlineColor(sf::Color::Blue);
    //             }
    //             if (!box.blockMove && box.blockVision)
    //             {
    //                 rect.setOutlineColor(sf::Color::Red);
    //             }
    //             if (!box.blockMove && !box.blockVision)
    //             {
    //                 rect.setOutlineColor(sf::Color::White);
    //             }

    //             rect.setOutlineThickness(1);

    //             window.draw(rect);
    //         }

    //         if (e.hasComponent<CPatrol>())
    //         {
    //             auto& patrol = e.getComponent<CPatrol>().positions;
    //             for (size_t p = 0; p < patrol.size(); p++)
    //             {
    //                 dot.setPosition(patrol[p]);
    //                 window.draw(dot);
    //             }
    //         }

    //         if (e.hasComponent<CFollowPlayer>())
    //         {
    //             sf::VertexArray lines(sf::PrimitiveType::LineStrip, 2);
    //             lines[0].position.x = e.getComponent<CTransform>().pos.x;
    //             lines[0].position.y = e.getComponent<CTransform>().pos.y;
    //             lines[0].color = sf::Color::Black;
    //             lines[1].position.x = m_player.getComponent<CTransform>().pos.x;
    //             lines[1].position.y = m_player.getComponent<CTransform>().pos.y;
    //             lines[1].color = sf::Color::Black;
    //             window.draw(lines);
    //             dot.setPosition(e.getComponent<CFollowPlayer>().home);
    //             window.draw(dot);
    //         }
    //     }
    // }

    /// fps counter

    float elapsedTime = m_fpsClock.restart().asSeconds();
    float fps = 1.0f / elapsedTime;
    m_fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));

    // draw the fps text on the default view (w.r.t. window coordinates, not game world)
    window.setView(window.getDefaultView());
    window.draw(m_fpsText);

    /// light cone
    /// TODO:



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
Vec2f ScenePlay::gridToMidPixel(const float gridX, const float gridY, const Entity entity)
{
    PROFILE_FUNCTION();

    const Vec2i& entityAnimSize = entity.getComponent<CAnimation>().animation.getSize();

    float xPos = gridX * m_cellSizePixels.x + entityAnimSize.x / 2.0f;
    float yPos = gridY * m_cellSizePixels.y + entityAnimSize.y / 2.0f;

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
    m_player.addComponent<CTransform>(gridToMidPixel(m_worldMaxCells.x / 2, m_playerConfig.GY, m_player));
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
    float bulletSpeed = 10.0f;

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
    bullet.addComponent<CLifespan>(60, m_currentFrame);
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
    int playerGridPosX = playerTrans.pos.x / m_cellSizePixels.x;
    int playerGridPosY = playerTrans.pos.y / m_cellSizePixels.y;
    int horizontalCheckLength = playerBounds.halfSize.x / m_cellSizePixels.x + 1;
    int verticalCheckLenght = playerBounds.halfSize.y / m_cellSizePixels.y + 1;
    for (int x = playerGridPosX - horizontalCheckLength; x <= playerGridPosX + horizontalCheckLength; ++x)
    {
        /// TODO: this may be faster using std::clamp in the loop arguments
        if (x < 0 || x >= m_worldMaxCells.x)
        {
            continue;
        }

        for (int y = playerGridPosY - verticalCheckLenght; y <= playerGridPosY + verticalCheckLenght; ++y)
        {
            /// TODO: this may be faster using std::clamp in the loop arguments
            if (y < 0 || y >= m_worldMaxCells.y)
            {
                continue;
            }

            if (tileMatrix[x][y].isActive()) /// TODO: accessing memory in tileMatrix and then switching to entity memory pool, might want local var in tileMatrix or somethin so we don't have to do this
            {
                Vec2f overlap = Physics::GetOverlap(m_player, tileMatrix[x][y]);

                // there is a collision
                if (overlap.y > 0 && overlap.x > 0)
                {
                    collision = true;
                    Vec2f prevOverlap = Physics::GetPreviousOverlap(m_player, tileMatrix[x][y]);

                    // we are colliding in y-direction this frame since previous frame already had x-direction overlap
                    if (prevOverlap.x > 0)
                    {
                        // player moving down
                        if (playerTrans.velocity.y > 0)
                        {
                            playerTrans.pos.y -= overlap.y; // player can't fall below tile
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
                            playerTrans.pos.y += overlap.y;
                        }
                        playerTrans.velocity.y = 0;
                    }

                    // colliding in x-direction this frame
                    if (prevOverlap.y > 0)
                    {
                        // player moving right
                        if (playerTrans.velocity.x > 0)
                        {
                            playerTrans.pos.x -= overlap.x;
                        }
                        // player moving left
                        else if (playerTrans.velocity.x < 0)
                        {
                            playerTrans.pos.x += overlap.x;
                        }
                        playerTrans.velocity.x = 0;
                    }
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

        /// TODO: consider adding a bounding box check for bullets (or just leave them as one pixel at the tip of the bullet so I never have to check)
        // int horizontalCheckLength = bulletBounds.halfSize.x / m_cellSizePixels.x + 1;
        // int verticalCheckLenght = bulletBounds.halfSize.y / m_cellSizePixels.y + 1;

        /// TODO: if no bounding box, don't even need any of this loop, can just get bullet pos (pixels), see if tile active at grid coord, then say there's a collision and handle it, no isinside or overlap or anything either

        if (bulletGridPosX < 0 || bulletGridPosX >= m_worldMaxCells.x || bulletGridPosY < 0 || bulletGridPosY >= m_worldMaxCells.y)
        {
            continue;
        }

        const Entity& tile = tileMatrix[bulletGridPosX][bulletGridPosY];

        if (tile.isActive()) /// TODO: accessing memory in tileMatrix and then switching to entity memory pool, might want local var in tileMatrix or somethin so we don't have to do this
        {
            int& bDamage = bullet.getComponent<CDamage>().damage;
            int& tHealth = tile.getComponent<CHealth>().current;

            tHealth -= bDamage;
            bDamage /= 2;

            if (bDamage <= 0)
            {
                bullet.destroy();
            }

            /// TODO: could create a toDestory vector and destroy entities all at once somewhere else if faster, test, since we are accessing tileMatrix and then memory pool back and forth for all bullets
            if (tHealth <= 0)
            {
                tile.destroy();

                /// TODO: check neighbors for floating tiles, then apply physics to them (if no close background)
            }
        }

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
    // move existing projectiles
    /// TODO: works for bullets, change when adding more projectile types
    for (Entity& p : projectiles)
    {
        CTransform& pTrans = p.getComponent<CTransform>();
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