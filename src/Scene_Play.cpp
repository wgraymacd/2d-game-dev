#include "Timer.hpp"

#include "Scene_Play.hpp"
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
#include <string>
#include <fstream>

/// @brief vonstructs a new Scene_Play object, calls Scene_Play::init
/// @param gameEngine the game's main engine which handles scene switching and adding, and other top-level functions; required by Scene to set m_game
/// @param levelPath the file path to the scene's configuration file
Scene_Play::Scene_Play(GameEngine& gameEngine)
    : Scene(gameEngine)
{
    PROFILE_FUNCTION();

    init();
}

/// @brief initializes the scene: registers keybinds, sets grid and fps text attributes, and calls loadGame
void Scene_Play::init()
{
    PROFILE_FUNCTION();

    // misc keybind setup
    registerAction(static_cast<int>(sf::Keyboard::Key::P), "PAUSE");
    registerAction(static_cast<int>(sf::Keyboard::Key::Escape), "QUIT");
    registerAction(static_cast<int>(sf::Keyboard::Key::T), "TOGGLE_TEXTURE");
    registerAction(static_cast<int>(sf::Keyboard::Key::C), "TOGGLE_COLLISION");
    registerAction(static_cast<int>(sf::Keyboard::Key::G), "TOGGLE_GRID");

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

    loadGame();
}

/// @brief calculates the midpoint of entity based on a given grid position
/// @param gridX entity's grid x coordinate
/// @param gridY entity's grid y coordinate
/// @param entity an entity in the scene
/// @return a Vec2f with the x and y pixel coordinates of the center of entity
Vec2f Scene_Play::gridToMidPixel(float gridX, float gridY, Entity entity)
{
    PROFILE_FUNCTION();

    const Vec2i& entityAnimSize = entity.getComponent<CAnimation>().animation.getSize();

    float xPos = gridX * m_gridSize.x + entityAnimSize.x / 2.0f;
    float yPos = gridY * m_gridSize.y + entityAnimSize.y / 2.0f;

    return Vec2f(xPos, yPos);
}

// return Vec2f indicating where the pos (top-left corner) of the entity should be, (0, 0) is top-left corner of window
// Vec2f Scene_Play::gridToPixel(float gridX, float gridY)
// {
//     float xPos = gridX * m_gridSize.x;
//     float yPos = gridY * m_gridSize.y;
// }

/// TODO: consider keeping this for later in case people want to save level, then can load with this function
/// @brief loads the scene using the configuration file levelPath
/// @param levelPath the configuration file specifying various components of entities in the scene
void Scene_Play::loadGame()
{
    PROFILE_FUNCTION();

    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    // read in the level file and add the appropriate entities
    std::ifstream file("../bin/playerConfig.txt");

    if (!file.is_open())
    {
        std::cerr << "Level file could not be opened: " << "../bin/playerConfig.txt" << std::endl;
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
void Scene_Play::generateWorld()
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

    /// generate world and get tile positions
    WorldGenerator gen(m_worldMax.x / m_gridSize.x, m_worldMax.y / m_gridSize.y);
    gen.generateWorld();
    const std::vector<TileInfo>& tilePositions = gen.getTilePositions();

    /// spawn tiles according to their positions in the grid
    for (const TileInfo& info : tilePositions)
    {
        Entity tile = m_entityManager.addEntity("tile");
        tile.addComponent<CAnimation>(m_game.assets().getAnimation(info.type), true);
        tile.addComponent<CTransform>(gridToMidPixel(info.x, info.y, tile));
        tile.addComponent<CBoundingBox>(m_game.assets().getAnimation(info.type).getSize());
        tile.addComponent<CHealth>(info.type == "dirt" ? 5 : 10, info.type == "dirt" ? 5 : 10);
    }
}

/// @brief spawns the player entity
void Scene_Play::spawnPlayer()
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
    m_player.addComponent<CAnimation>(m_game.assets().getAnimation("BlockGrass1"), true);
    m_player.addComponent<CTransform>(gridToMidPixel(m_playerConfig.GX, m_playerConfig.GY, m_player));
    m_player.addComponent<CBoundingBox>(Vec2i(m_playerConfig.CW, m_playerConfig.CH));
    m_player.addComponent<CState>("air");
    m_player.addComponent<CInput>();
    m_player.addComponent<CGravity>(m_playerConfig.GRAVITY);
}

/// @brief spawn a bullet at the location of entity traveling toward cursor
void Scene_Play::spawnBullet(Entity entity)
{
    PROFILE_FUNCTION();

    Vec2f& entityPos = entity.getComponent<CTransform>().pos;
    float bulletSpeed = 30.0f;

    const Vec2f& worldTarget = m_game.window().mapPixelToCoords(sf::Mouse::getPosition(m_game.window()));

    Entity bullet = m_entityManager.addEntity("bullet");
    bullet.addComponent<CAnimation>(m_game.assets().getAnimation(m_playerConfig.BA), true);
    bullet.addComponent<CTransform>
        (
            entityPos,
            (worldTarget - entityPos) * bulletSpeed / worldTarget.dist(entityPos),
            Vec2f(1.0f, 1.0f), 0.0f
        );
    bullet.addComponent<CBoundingBox>(bullet.getComponent<CAnimation>().animation.getSize());
    bullet.addComponent<CLifespan>(30, m_currentFrame);
    bullet.addComponent<CDamage>(2);
}

/// @brief update the scene; this function is called by the game engine at each frame if this scene is active
void Scene_Play::update()
{
    PROFILE_FUNCTION();

    if (!m_paused)
    {
        /// TODO: think about order here if it even matters
        sAI();
        sMovement();
        sStatus();
        sCollision(); // after sMovement
        sAnimation();
        sCamera();

        m_entityManager.update(); // adding and removing all entities based on sCollision, spawnBullet, etc.
    }

    sRender();
}

/// @brief handle player and bullet movement per frame
void Scene_Play::sMovement()
{
    PROFILE_FUNCTION();

    /// player

    std::string& state = m_player.getComponent<CState>().state;
    CInput& input = m_player.getComponent<CInput>();
    CTransform& trans = m_player.getComponent<CTransform>();

    Vec2f velToAdd(0.0f, 0.0f);

    /// TODO: consider turing this into real physics
    // gravity
    float airResistance = 15.0f;
    if (trans.velocity.y + m_playerConfig.GRAVITY >= airResistance)
    {
        velToAdd.y += airResistance - trans.velocity.y;
    }
    else
    {
        velToAdd.y += m_playerConfig.GRAVITY;
    }

    // no left or right input - slow down in x-direction (less if in air, more if on ground) until stopped
    if (!input.left && !input.right)
    {
        // float friction = m_playerConfig.SX / 2.0f;
        // set friction value based on state
        float friction;
        if (state == "air")
        {
            friction = 0.2f;
        }
        else // if (state == "run")
        {
            friction = 1.0f;
        }

        // slow down until stopped
        if (abs(trans.velocity.x) >= friction)
        {
            velToAdd.x += (trans.velocity.x > 0 ? -friction : friction);
        }
        else
        {
            velToAdd.x -= trans.velocity.x;
        }
    }

    if (input.right)
    {
        if (trans.velocity.x + m_playerConfig.SX <= m_playerConfig.SM)
        {
            velToAdd.x += m_playerConfig.SX;
        }
        else
        {
            velToAdd.x = m_playerConfig.SM - trans.velocity.x;
        }

        trans.scale.x = abs(trans.scale.x);
        /// TODO: overrite if shooting in other direction (here or maybe in spawnBullet or something)
    }

    if (input.left)
    {
        if (trans.velocity.x - m_playerConfig.SX >= -m_playerConfig.SM)
        {
            velToAdd.x -= m_playerConfig.SX;
        }
        else
        {
            velToAdd.x = -m_playerConfig.SM - trans.velocity.x;
        }

        trans.scale.x = -abs(trans.scale.x);
        /// TODO: overrite if shooting in other direction (here or maybe in spawnBullet or something)
    }

    if (input.up && input.canJump)
    {
        velToAdd.y -= m_playerConfig.SY;
        input.canJump = false; // set to true in sCollision (must see if on the ground)
    }

    // on release of jump key
    /// TODO: implement new jumping (min jump height, no sudden fall on release, double jumping / flying)
    if (!input.up && trans.velocity.y < 0)
    {
        trans.velocity.y = 0;
    }

    trans.velocity += velToAdd;
    trans.prevPos = trans.pos;
    trans.pos += trans.velocity;

    /// TODO: have crouching? does this then go in sUserInput?
    // if (trans.velocity.x == 0 && trans.velocity.y == 0)
    // {
    //     if (input.down)
    //     {
    //         state = "crouch";
    //         std::cout << "state set to: " << state << std::endl;
    //     }
    //     else
    //     {
    //         state = "stand";
    //         std::cout << "state set to: " << state << std::endl;
    //     }
    // }

    /// bullets

    for (auto& bullet : m_entityManager.getEntities("bullet"))
    {
        CTransform& trans = bullet.getComponent<CTransform>();
        trans.pos += trans.velocity;
    }
    if (input.shoot && input.canShoot)
    {
        spawnBullet(m_player);
    }
}

/// TODO: modularize some of this if needed to reduce repition and make it easier to read
/// TODO: increase efficiency with chunking or something like that, maybe a distance check or an in-frame/in-window check if possible
/// @brief handle collisions and m_player CState updates
void Scene_Play::sCollision()
{
    PROFILE_FUNCTION();

    CTransform& trans = m_player.getComponent<CTransform>();
    std::string& state = m_player.getComponent<CState>().state;

    bool collision = false;

    // cache these once per frame
    const std::vector<Entity>& tiles = m_entityManager.getEntities("tile");
    const std::vector<Entity>& bullets = m_entityManager.getEntities("bullet");

    for (const Entity& tile : tiles)
    {
        PROFILE_SCOPE("player/bullet-tile");

        Vec2f overlap = Physics::GetOverlap(m_player, tile);

        // there is a collision
        if (overlap.y > 0 && overlap.x > 0)
        {
            collision = true;
            Vec2f prevOverlap = Physics::GetPreviousOverlap(m_player, tile);

            // we are colliding in y-direction this frame since previous frame already had x-direction overlap
            if (prevOverlap.x > 0)
            {
                // player moving down
                if (trans.velocity.y > 0)
                {
                    trans.pos.y -= overlap.y; // player can't fall below tile
                    state = abs(trans.velocity.x) > 0 ? "run" : "stand";

                    // jumping
                    if (!m_player.getComponent<CInput>().up) // wait for player to release w key before allowing jump
                    {
                        m_player.getComponent<CInput>().canJump = true; // set to false after jumping in sMovement()
                    }
                }

                // player moving up
                else if (trans.velocity.y < 0)
                {
                    trans.pos.y += overlap.y;
                }
                trans.velocity.y = 0;
            }

            // colliding in x-direction this frame
            if (prevOverlap.y > 0)
            {
                // player moving right
                if (trans.velocity.x > 0)
                {
                    trans.pos.x -= overlap.x;
                }
                // player moving left
                else if (trans.velocity.x < 0)
                {
                    trans.pos.x += overlap.x;
                }
                trans.velocity.x = 0;
            }
        }

        for (const Entity& bullet : bullets)
        {
            PROFILE_SCOPE("bullet-tile");

            // treating bullets as small rectangles to be able to use same Physics::GetOverlap function
            Vec2f overlap = Physics::GetOverlap(tile, bullet);

            if (overlap.x > 0 && overlap.y > 0)
            {
                // std::cout << "tile " << tile->id() << "(" << tile->get<CTransform>().pos.x << ", " << tile->get<CTransform>().pos.y << ")" << " and bullet " << bullet->id() << "(" << bullet->get<CTransform>().pos.x << ", " << bullet->get<CTransform>().pos.y << ")" << " collided" << std::endl;
                /// TODO: do whatever else here I might want (animations, tile weakening, etc.)
                bullet.destroy();
                tile.getComponent<CHealth>().current -= bullet.getComponent<CDamage>().damage;
            }
        }
    }

    if (!collision)
    {
        state = "air";
    }

    // player falls below map
    // if (trans.pos.y - m_player->get<CAnimation>().animation.getSize().y / 2 > m_game.window().getSize().y)
    // {
    //     spawnPlayer();
    // }

    // restrict movement passed top, bottom, or side of map
    const Vec2i& animSize = m_player.getComponent<CAnimation>().animation.getSize();
    if (trans.pos.x < animSize.x / 2)
    {
        trans.pos.x = animSize.x / 2;
        trans.velocity.x = 0;
    }
    else if (trans.pos.x > m_worldMax.x - animSize.x / 2)
    {
        trans.pos.x = m_worldMax.x - animSize.x / 2;
        trans.velocity.x = 0;
    }
    else if (trans.pos.y < animSize.y / 2)
    {
        trans.pos.y = animSize.y / 2;
        trans.velocity.y = 0;
    }
    else if (trans.pos.y > m_worldMax.y - animSize.y / 2)
    {
        trans.pos.y = m_worldMax.y - animSize.y / 2;
        trans.velocity.y = 0;
        m_player.getComponent<CInput>().canJump = true;
    }
}

/// TODO: grouping similar actions (e.g., input actions like "JUMP", "LEFT", "RIGHT", etc.) into an enum or constants to avoid potential typos and improve maintainability. This way, your if-else chains would be more scalable if new actions are added
/// @brief sets CInput variables according to action, no action logic here
/// @param action an action to perform; action has a type and a name which are both std::string objects
void Scene_Play::sDoAction(const Action& action)
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
void Scene_Play::sAI()
{
    PROFILE_FUNCTION();
}

/// TODO: finish this
/// @brief updates all entities' lifespan and whatever else status
void Scene_Play::sStatus()
{
    PROFILE_FUNCTION();

    for (auto& e : m_entityManager.getEntities())
    {
        // lifespan
        if (e.hasComponent<CLifespan>())
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

        // health
        if (e.hasComponent<CHealth>())
        {
            int& health = e.getComponent<CHealth>().current;
            if (health <= 0)
            {
                e.destroy();
            }
        }
    }
}

/// @brief handles all entities' animation updates
void Scene_Play::sAnimation()
{
    PROFILE_FUNCTION();

    /// TODO: Complete the Animation class code first
    // for each entity with an animation, call entity->get<CAnimation>().animation.update()
    // if animation is not repeated, and it has ended, destroy the entity

    // set animation of player based on its CState component
}

/// @brief handles camera view logic
void Scene_Play::sCamera()
{
    PROFILE_FUNCTION();

    sf::View view = m_game.window().getView();
    Vec2f& pPos = m_player.getComponent<CTransform>().pos;

    // center the view on the player
    /// TODO: deal with the edge of world view, prolly make it so that view center is always at player center for competitive fairness and less disorientation, will need a way to keep players inside world bound tho (force or invisible wall with background or darkness or just more tiles that extend out of sight, could also throw in some easter eggs / secrets there
    const Vec2ui& viewSize = m_game.window().getSize();
    float viewCenterX = std::clamp(pPos.x, viewSize.x / 2.0f, m_worldMax.x - viewSize.x / 2.0f);
    float viewCenterY = std::clamp(pPos.y, viewSize.y / 2.0f, m_worldMax.y - viewSize.y / 2.0f);
    view.setCenter({ viewCenterX, viewCenterY });

    // move the camera slightly toward the players mouse position (capped at a max displacement)
    const Vec2i& mousePosOnWindow = sf::Mouse::getPosition(m_game.window());
    float dx = std::clamp((mousePosOnWindow.x - viewSize.x / 2.0f) * 0.15f, -25.0f, 25.0f);
    float dy = std::clamp((mousePosOnWindow.y - viewSize.y / 2.0f) * 0.15f, -25.0f, 25.0f);
    view.move({ dx, dy });

    // set window view
    m_game.window().setView(view);
}

/// @brief changes back to MENU scene when this scene ends
void Scene_Play::onEnd()
{
    PROFILE_FUNCTION();

    m_game.changeScene("MENU");

    /// TODO: stop music, play menu music
}

/// @brief handles all rendering of textures (animations), grid boxes, collision boxes, and fps counter
void Scene_Play::sRender()
{
    PROFILE_FUNCTION();

    // color the background darker so you know that the game is paused
    if (!m_paused)
    {
        m_game.window().clear(sf::Color(5, 5, 5));
    }
    else
    {
        m_game.window().clear(sf::Color(10, 10, 10));
    }

    std::vector<Entity>& entities = m_entityManager.getEntities();

    /// draw all entity textures / animations

    if (m_drawTextures)
    {
        for (auto e : entities)
        {
            CTransform& transform = e.getComponent<CTransform>();

            if (e.hasComponent<CAnimation>())
            {
                sf::Sprite& sprite = e.getComponent<CAnimation>().animation.getSprite();
                sprite.setRotation(sf::radians(transform.rotAngle));
                sprite.setPosition(transform.pos);
                sprite.setScale(transform.scale);

                m_game.window().draw(sprite);
            }
        }

        /// grid

        if (m_drawGrid)
        {
            float leftX = m_game.window().getView().getCenter().x - (m_game.window().getView().getSize().x / 2);
            float rightX = leftX + m_game.window().getView().getSize().x; // + m_gridSize.x maybe
            // logic if grid (0, 0) at bottom left
            // float bottomY = m_game.window().getView().getCenter().y + (m_game.window().getView().getSize().y / 2);
            // float topY = bottomY - m_game.window().getView().getSize().y;
            // logic if grid (0, 0) at top left
            float topY = m_game.window().getView().getCenter().y - m_game.window().getView().getSize().y / 2;
            float bottomY = topY + m_game.window().getView().getSize().y; // + m_gridSize.y maybe

            float nextGridX = leftX - fmodf(leftX, m_gridSize.x);
            // logic if grid (0, 0) at bottom left
            // float nextGridY = bottomY - fmodf(bottomY, m_gridSize.y);
            // logic if grid (0, 0) at top left
            float nextGridY = topY - fmodf(topY, m_gridSize.y);

            // vertical grid lines
            for (float x = nextGridX; x <= rightX; x += m_gridSize.x)
            {
                drawLine(Vec2f(x, topY), Vec2f(x, bottomY));
            }

            // horizontal grid lines
            // logic if grid (0, 0) at bottom left
            // for (float y = nextGridY; y >= topY; y -= m_gridSize.y)
            // logic if grid (0, 0) at top left
            for (float y = nextGridY; y <= bottomY; y += m_gridSize.y)
            {
                drawLine(Vec2f(leftX, y), Vec2f(rightX, y));

                // grid cell labels
                for (float x = nextGridX; x <= rightX; x += m_gridSize.x)
                {
                    std::string xCell = std::to_string(static_cast<int>(x / m_gridSize.x));
                    // logic if grid (0, 0) at bottom left
                    // std::string yCell = std::to_string(static_cast<int>((bottomY - y) / m_gridSize.y));
                    // logic if grid (0, 0) at top left
                    std::string yCell = std::to_string(static_cast<int>(y / m_gridSize.y));

                    m_gridText.setString("(" + xCell + "," + yCell + ")");
                    // m_gridText.setPosition(x + 3, y - m_gridSize.y + 2); // position label inside cell, bottom left (0, 0)
                    m_gridText.setPosition({ x + 3, y + 2 }); // position label inside cell, top left (0, 0)
                    m_gridText.setFillColor(sf::Color(255, 255, 255, 50));
                    m_game.window().draw(m_gridText);
                }
            }
        }
    }

    /// draw all entity collision bounding boxes with a rectangle

    if (m_drawCollision)
    {
        sf::CircleShape dot(4);
        dot.setFillColor(sf::Color::Black);
        for (auto e : m_entityManager.getEntities())
        {
            if (e.hasComponent<CBoundingBox>())
            {
                auto& box = e.getComponent<CBoundingBox>();
                auto& transform = e.getComponent<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(Vec2f(box.size.x - 1, box.size.y - 1)); // - 1 cuz line thickness of 1?
                rect.setOrigin(box.halfSize);
                rect.setPosition(transform.pos);
                rect.setFillColor(sf::Color(0, 0, 0, 0));
                rect.setOutlineColor(sf::Color(255, 255, 255, 255));

                if (box.blockMove && box.blockVision)
                {
                    rect.setOutlineColor(sf::Color::Black);
                }
                if (box.blockMove && !box.blockVision)
                {
                    rect.setOutlineColor(sf::Color::Blue);
                }
                if (!box.blockMove && box.blockVision)
                {
                    rect.setOutlineColor(sf::Color::Red);
                }
                if (!box.blockMove && !box.blockVision)
                {
                    rect.setOutlineColor(sf::Color::White);
                }

                rect.setOutlineThickness(1);

                m_game.window().draw(rect);
            }

            if (e.hasComponent<CPatrol>())
            {
                auto& patrol = e.getComponent<CPatrol>().positions;
                for (size_t p = 0; p < patrol.size(); p++)
                {
                    dot.setPosition(patrol[p]);
                    m_game.window().draw(dot);
                }
            }

            if (e.hasComponent<CFollowPlayer>())
            {
                sf::VertexArray lines(sf::PrimitiveType::LineStrip, 2);
                lines[0].position.x = e.getComponent<CTransform>().pos.x;
                lines[0].position.y = e.getComponent<CTransform>().pos.y;
                lines[0].color = sf::Color::Black;
                lines[1].position.x = m_player.getComponent<CTransform>().pos.x;
                lines[1].position.y = m_player.getComponent<CTransform>().pos.y;
                lines[1].color = sf::Color::Black;
                m_game.window().draw(lines);
                dot.setPosition(e.getComponent<CFollowPlayer>().home);
                m_game.window().draw(dot);
            }
        }
    }

    /// fps counter

    float elapsedTime = m_fpsClock.restart().asSeconds();
    float fps = 1.0f / elapsedTime;
    m_fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));

    // draw the fps text on the default view (w.r.t. window coordinates, not game world)
    sf::View currentView = m_game.window().getView();
    m_game.window().setView(m_game.window().getDefaultView());
    m_game.window().draw(m_fpsText);
    m_game.window().setView(currentView);

    /// light cone
    /// TODO:

    m_game.window().display();
}

/// @brief helper function for grid drawing; draws line from p1 to p2 on the screen
/// @param p1 first point in line
/// @param p2 second point in line
void Scene_Play::drawLine(const Vec2f& p1, const Vec2f& p2)
{
    sf::Vertex line[] =
    {
        {p1, sf::Color(255, 255, 255, 50)},
        {p2, sf::Color(255, 255, 255, 50)}
    };

    m_game.window().draw(line, 2, sf::PrimitiveType::Lines);
}
