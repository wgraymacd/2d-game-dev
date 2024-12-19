#include "Scene_Play.h"
#include "Scene.h"
#include "GameEngine.h"
#include "EntityManager.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include "Vec2.hpp"
#include "Physics.hpp"
#include "Animation.hpp"
#include "Action.hpp"

#include <SFML/Graphics.hpp>

#include <string>
#include <fstream>
#include <memory>

/// @brief vonstructs a new Scene_Play object, calls Scene_Play::init
/// @param gameEngine the game's main engine which handles scene switching and adding, and other top-level functions; required by Scene to set m_game
/// @param levelPath the file path to the scene's configuration file
Scene_Play::Scene_Play(GameEngine &gameEngine, const std::string &levelPath)
    : Scene(gameEngine), m_levelPath(levelPath)
{
    init(levelPath);
}

/// @brief initializes the scene: registers keybinds, sets grid and fps text attributes, and calls loadLevel
/// @param levelPath the file path to the scene's configuration file; passed to loadLevel
void Scene_Play::init(const std::string &levelPath)
{
    // misc keybind setup
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");

    // player keyboard setup
    registerAction(sf::Keyboard::W, "JUMP");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");

    // player mouse setup
    registerAction(sf::Mouse::Button::Left, "SHOOT", true);

    // grid text setup
    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game.assets().getFont("PixelCowboy"));

    // fps counter setup
    m_fpsText.setFont(m_game.assets().getFont("PixelCowboy"));
    m_fpsText.setCharacterSize(12);
    m_fpsText.setFillColor(sf::Color::White);
    m_fpsText.setPosition(10.f, 10.f); // top-left corner

    loadLevel(levelPath);
}

/// @brief calculates the midpoint of entity based on a given grid position
/// @param gridX entity's grid x coordinate
/// @param gridY entity's grid y coordinate
/// @param entity an entity in the scene
/// @return a Vec2f with the x and y pixel coordinates of the center of entity
Vec2f Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
    float xPos = gridX * m_gridSize.x + entity->get<CAnimation>().animation.getSize().x / 2.0f;
    float yPos = m_game.window().getSize().y - gridY * m_gridSize.y - entity->get<CAnimation>().animation.getSize().y / 2.0f;

    return Vec2f(xPos, yPos);
}

// return Vec2f indicating where the pos (top-left corner) of the entity should be, (0, 0) is top-left corner of window
// Vec2f Scene_Play::gridToPixel(float gridX, float gridY)
// {
//     float xPos = gridX * m_gridSize.x;
//     float yPos = gridY * m_gridSize.y;
// }

/// @brief loads the scene using the configuration file levelPath
/// @param levelPath the configuration file specifying various components of entities in the scene
void Scene_Play::loadLevel(const std::string &levelPath)
{
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    // read in the level file and add the appropriate entities
    std::ifstream file(levelPath);

    if (!file.is_open())
    {
        std::cerr << "Level file could not be opened: " << levelPath << std::endl;
        exit(-1);
    }

    std::string type;

    while (file >> type)
    {
        // TODO: consider position decorations w.r.t. their top-left corner using gridToPixel instead of center (things with collisions implemented with centered positions)
        if (type == "Tile")
        {
            std::shared_ptr<Entity> tile = m_entityManager.addEntity("tile");

            // add CAnimation component first so that gridToMidPixel works
            std::string animation;
            file >> animation;
            tile->add<CAnimation>(m_game.assets().getAnimation(animation), true);

            float gridX, gridY;
            file >> gridX >> gridY;
            tile->add<CTransform>(gridToMidPixel(gridX, gridY, tile));

            tile->add<CBoundingBox>(m_game.assets().getAnimation(animation).getSize());

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

    spawnPlayer();
}

/// @brief spawns the player entity
void Scene_Play::spawnPlayer()
{
    // when respawning in same map, must delete previous player
    if (!m_entityManager.getEntities("player").empty())
    {
        m_entityManager.getEntities("player").front()->destroy();
    }

    // set player components
    m_player = m_entityManager.addEntity("player");
    m_player->add<CAnimation>(m_game.assets().getAnimation("GroundBlack"), true);
    m_player->add<CTransform>(gridToMidPixel(m_playerConfig.GX, m_playerConfig.GY, m_player));
    m_player->add<CBoundingBox>(Vec2f(m_playerConfig.CW, m_playerConfig.CH));
    m_player->add<CState>("stand");
    m_player->add<CInput>();
    m_player->add<CGravity>(m_playerConfig.GRAVITY);
}

/// @brief spawn a bullet at the location of entity traveling toward cursor
/// @param entity an entity in the scene
void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
    Vec2f &entityPos = entity->get<CTransform>().pos;
    float bulletSpeed = 30.0f;
    const Vec2i &target = sf::Mouse::getPosition(m_game.window());

    std::shared_ptr<Entity> bullet = m_entityManager.addEntity("bullet");
    bullet->add<CAnimation>(m_game.assets().getAnimation(m_playerConfig.BA), true);
    bullet->add<CTransform>(entityPos, (target.to<float>() - entityPos) * bulletSpeed / target.to<float>().dist(entityPos), Vec2f(1.0f, 1.0f), 0.0f);
    bullet->add<CBoundingBox>(bullet->get<CAnimation>().animation.getSize());
    bullet->add<CLifespan>(30, m_currentFrame); // TODO: Lifespan component could use some cleanup, along with everything in general once finished with functionality (didn't end up using everything the way Dave set it up)
}

/// @brief spawn a melee attack in front of entity
/// @param entity an entity in the scene
void Scene_Play::spawnMelee(std::shared_ptr<Entity> entity)
{
    // TODO: spawn a sword or a knife for melee attacks
}

/// @brief update the scene; this function is called by the game engine at each frame if this scene is active
void Scene_Play::update()
{
    if (!m_paused)
    {
        // TODO: think about order here if it even matters
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
    /* player */
    std::string &state = m_player->get<CState>().state;
    CInput &input = m_player->get<CInput>();
    CTransform &trans = m_player->get<CTransform>();

    Vec2f velToAdd(0, 0);

    velToAdd.y += m_playerConfig.GRAVITY;

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

        /// TODO: something strange happening with friction after landing (steady movement to the left on the ground for a brief time)
        // slow down until stopped
        if (abs(trans.velocity.x) >= friction)
        {
            velToAdd.x += (trans.velocity.x > 0 ? -friction : friction);
        }
        else
        {
            velToAdd.x = (trans.velocity.x > 0 ? -trans.velocity.x : trans.velocity.x);
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

        trans.scale.x = abs(trans.scale.x); // TODO: overrite if shooting in other direction (here or maybe in spawnBullet or something)
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

        trans.scale.x = -abs(trans.scale.x); // TODO: overrite if shooting in other direction (here or maybe in spawnBullet or something)
    }

    if (input.up && input.canJump)
    {
        velToAdd.y -= m_playerConfig.SY;
        input.canJump = false; // set to true in sCollision (must see if on the ground)
    }

    // on release of jump key
    // TODO: implement new jumping (min jump height, no sudden fall on release, double jumping / flying)
    if (!input.up && trans.velocity.y < 0)
    {
        trans.velocity.y = 0;
    }

    trans.velocity += velToAdd;
    trans.prevPos = trans.pos;
    trans.pos += trans.velocity;

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

    /* bullets */
    for (auto &bullet : m_entityManager.getEntities("bullet"))
    {
        CTransform &trans = bullet->get<CTransform>();
        trans.pos += trans.velocity;
    }
    if (input.shoot && input.canShoot) 
    {
        spawnBullet(m_player);
    }
}

/// @brief handle collisions and m_player CState updates
void Scene_Play::sCollision()
{
    CTransform &trans = m_player->get<CTransform>();
    std::string &state = m_player->get<CState>().state;

    bool collision = false;

    /* player and tiles */
    for (auto &tile : m_entityManager.getEntities("tile"))
    {
        Vec2f overlap = Physics::GetOverlap(m_player, tile);
        Vec2f prevOverlap = Physics::GetPreviousOverlap(m_player, tile);

        // there is a collision
        if (overlap.y > 0 && overlap.x > 0)
        {
            collision = true;

            // we are colliding in y-direction this frame since previous frame already had x-direction overlap
            if (prevOverlap.x > 0)
            {
                // player moving down
                if (trans.velocity.y > 0)
                {
                    trans.pos.y -= overlap.y; // player can't fall below tile
                    state = abs(trans.velocity.x) > 0 ? "run" : "stand";
                    m_player->get<CInput>().canJump = true; // set to false after jumping in sMovement()
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
    }

    if (!collision)
    {
        state = "air";
    }

    // TODO: put inner for loop inside the one above? keep like this to isolate scopes?
    // something wrong with bullet collision when using a scale that is not 1:1 (and maybe in general too)
    for (auto &tile : m_entityManager.getEntities("tile"))
    {
        for (auto &bullet : m_entityManager.getEntities("bullet"))
        {
            // treating bullets as small rectangles to be able to use same Physics::GetOverlap function
            Vec2f overlap = Physics::GetOverlap(tile, bullet);

            if (overlap.x > 0 && overlap.y > 0)
            {
                // std::cout << "tile " << tile->id() << "(" << tile->get<CTransform>().pos.x << ", " << tile->get<CTransform>().pos.y << ")" << " and bullet " << bullet->id() << "(" << bullet->get<CTransform>().pos.x << ", " << bullet->get<CTransform>().pos.y << ")" << " collided" << std::endl;
                // TODO: do whatever else here I might want (animations, tile weakening, etc.)
                bullet->destroy();
            }
        }
    }

    // player falls below map
    if (trans.pos.y - m_player->get<CAnimation>().animation.getSize().y / 2 > m_game.window().getSize().y)
    {
        spawnPlayer();
    }

    // side of map
    Animation &anim = m_player->get<CAnimation>().animation;
    if (trans.pos.x < anim.getSize().x / 2)
    {
        trans.pos.x = anim.getSize().x / 2;
        trans.velocity.x = 0;
    }
}

/// @brief sets CInput variables according to action, no action logic here
/// @param action an action to perform; action has a type and a name which are both std::string objects
void Scene_Play::sDoAction(const Action &action)
{
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
            m_player->get<CInput>().up = true;
        }
        else if (action.name() == "LEFT")
        {
            m_player->get<CInput>().left = true;
        }
        else if (action.name() == "RIGHT")
        {
            m_player->get<CInput>().right = true;
        }
        else if (action.name() == "SHOOT")
        {
            m_player->get<CInput>().shoot = true;
        }
        // else if (action.name() == "LEFT_CLICK")
        // {
        //     m_selectedEntity = {};
        //     sf::Vector2f worldPos = m_game.window().mapPixelToCoords(action.pos());
        //     for (auto e : m_entityManager.getEntities())
        //     {
        //         if (Physics::IsInside(worldPos, e))
        //         {
        //             m_selectedEntity = e;
        //             break;
        //         }
        //     }
        // }
    }
    else if (action.type() == "END")
    {
        if (action.name() == "JUMP")
        {
            m_player->get<CInput>().up = false;
        }
        else if (action.name() == "LEFT")
        {
            m_player->get<CInput>().left = false;
        }
        else if (action.name() == "RIGHT")
        {
            m_player->get<CInput>().right = false;
        }
        else if (action.name() == "SHOOT")
        {
            m_player->get<CInput>().shoot = false;
        }
    }
}

/// @brief handles the behavior of NPCs
void Scene_Play::sAI()
{
    // TODO: implement NPC AI follow and patrol behavior
}

// TODO: finish this
/// @brief updates all entities' lifespan and invincibility status
void Scene_Play::sStatus()
{
    // lifespan
    for (auto &e : m_entityManager.getEntities())
    {
        if (e->has<CLifespan>())
        {
            int &lifespan = e->get<CLifespan>().lifespan;
            if (lifespan <= 0)
            {
                e->destroy();
            }
            else
            {
                lifespan--;
            }
        }
    }

    // invincibility
}

/// @brief handles all entities' animation updates
void Scene_Play::sAnimation()
{
    // TODO: Complete the Animation class code first
    // for each entity with an animation, call entity->get<CAnimation>().animation.update()
    // if animation is not repeated, and it has ended, destroy the entity

    // set animation of player based on its CState component
    // here's an example
    if (m_player->get<CState>().state == "stand")
    {
        // change its animation to a repeating run animation
        // note: adding a component that already exists simple overwrites it
        m_player->add<CAnimation>(m_game.assets().getAnimation("GroundBlack"), true);
    }
}

/// @brief handles camera view logic
void Scene_Play::sCamera()
{
    // TODO: camera view logic

    // get current view
    sf::View view = m_game.window().getView();

    if (m_follow)
    {
        // calc view for player follow cam
    }
    else
    {
        // room-based cam
    }

    // OLD CODE: set the viewport of the window to be centered on the player if player is not on left bound of world
    // auto &pPos = m_player->get<CTransform>().pos;
    // float windowCenterX = std::max(m_game.window().getSize().x / 2.0f, pPos.x);
    // sf::View view = m_game.window().getView();
    // view.setCenter(windowCenterX, m_game.window().getSize().y / 2.0f);
    // m_game.window().setView(view);

    // set window view
    m_game.window().setView(view);
}

/// @brief changes back to MENU scene when this scene ends
void Scene_Play::onEnd()
{
    m_game.changeScene("MENU");

    // TODO: stop music, play menu music
}

/// @brief handles all rendering of textures (animations), grid boxes, collision boxes, and fps counter
void Scene_Play::sRender()
{
    // color the background darker so you know that the game is paused
    if (!m_paused)
    {
        m_game.window().clear(sf::Color(5, 5, 5));
    }
    else
    {
        m_game.window().clear(sf::Color(10, 10, 10));
    }

    sf::RectangleShape tick({1.0f, 6.0f});
    tick.setFillColor(sf::Color::Black);

    /* draw all entity textures / animations */
    if (m_drawTextures)
    {
        for (auto e : m_entityManager.getEntities()) // getEntities() returns reference
        {
            auto &transform = e->get<CTransform>();

            sf::Color c = sf::Color::White;
            if (e->has<CInvincibility>())
            {
                c = sf::Color(255, 255, 255, 128);
            }

            if (e->has<CAnimation>())
            {
                auto &animation = e->get<CAnimation>().animation;
                animation.getSprite().setRotation(transform.rotAngle);
                animation.getSprite().setPosition(transform.pos);
                animation.getSprite().setScale(transform.scale);
                animation.getSprite().setColor(c);

                m_game.window().draw(animation.getSprite());
            }
        }

        for (auto e : m_entityManager.getEntities())
        {
            auto &transform = e->get<CTransform>();
            if (e->has<CHealth>())
            {
                auto &h = e->get<CHealth>();
                Vec2f size(64, 6);
                sf::RectangleShape rect({size.x, size.y});
                rect.setPosition(transform.pos.x - 32, transform.pos.y - 48);
                rect.setFillColor(sf::Color(96, 96, 96));
                rect.setOutlineColor(sf::Color::Black);
                rect.setOutlineThickness(2);

                m_game.window().draw(rect);

                float ratio = static_cast<float>(h.current) / static_cast<float>(h.max);
                size.x *= ratio;
                rect.setSize({size.x, size.y});
                rect.setFillColor(sf::Color(255, 0, 0));
                rect.setOutlineThickness(0);
                
                m_game.window().draw(rect);

                for (int i = 0; i < h.max; i++)
                {
                    tick.setPosition(rect.getPosition().x + i * 64.0f / h.max, rect.getPosition().y);
                    m_game.window().draw(tick);
                }
            }
        }

        /* grid */
        if (m_drawGrid)
        {
            float leftX = m_game.window().getView().getCenter().x - (m_game.window().getView().getSize().x / 2);
            float rightX = leftX + m_game.window().getView().getSize().x; // + m_gridSize.x maybe
            float bottomY = m_game.window().getView().getCenter().y + (m_game.window().getView().getSize().y / 2);
            float topY = bottomY - m_game.window().getView().getSize().y;
            // logic if grid (0, 0) at top left
            // float topY = m_game.window().getView().getCenter().y - m_game.window().getView().getSize().y / 2;
            // float bottomY = topY + m_game.window().getView().getSize().y; // + m_gridSize.y maybe

            float nextGridX = leftX - fmodf(leftX, m_gridSize.x);
            float nextGridY = bottomY - fmodf(bottomY, m_gridSize.y);
            // logic if grid (0, 0) at top left
            // float nextGridY = topY - fmodf(topY, m_gridSize.y);

            // vertical grid lines
            for (float x = nextGridX; x <= rightX; x += m_gridSize.x)
            {
                drawLine(Vec2f(x, topY), Vec2f(x, bottomY));
            }

            // horizontal grid lines
            for (float y = nextGridY; y >= topY; y -= m_gridSize.y)
            // logic if grid (0, 0) at top left
            // for (float y = nextGridY; y <= bottomY; y += m_gridSize.y)
            {
                drawLine(Vec2f(leftX, y), Vec2f(rightX, y));

                // grid cell labels
                for (float x = nextGridX; x <= rightX; x += m_gridSize.x)
                {
                    std::string xCell = std::to_string(static_cast<int>(x / m_gridSize.x));
                    std::string yCell = std::to_string(static_cast<int>((bottomY - y) / m_gridSize.y));
                    // logic if grid (0, 0) at top left
                    // std::string yCell = std::to_string(static_cast<int>(y / m_gridSize.y));

                    m_gridText.setString("(" + xCell + "," + yCell + ")");
                    m_gridText.setPosition(x + 3, y - m_gridSize.y + 2); // position label inside cell
                    // top left: m_gridText.setPosition(x + 3, y + 2); // position label inside cell
                    m_gridText.setFillColor(sf::Color(255, 255, 255, 50));
                    m_game.window().draw(m_gridText);
                }
            }
        }
    }
    
    /* draw all entity collision bounding boxes with a rectangle */
    if (m_drawCollision)
    {
        sf::CircleShape dot(4);
        dot.setFillColor(sf::Color::Black);
        for (auto e : m_entityManager.getEntities())
        {
            if (e->has<CBoundingBox>())
            {
                auto &box = e->get<CBoundingBox>();
                auto &transform = e->get<CTransform>();
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

            if (e->has<CPatrol>())
            {
                auto &patrol = e->get<CPatrol>().positions;
                for (size_t p = 0; p < patrol.size(); p++)
                {
                    dot.setPosition(patrol[p].x, patrol[p].y);
                    m_game.window().draw(dot);
                }
            }

            if (e->has<CFollowPlayer>())
            {
                sf::VertexArray lines(sf::LinesStrip, 2);
                lines[0].position.x = e->get<CTransform>().pos.x;
                lines[0].position.y = e->get<CTransform>().pos.y;
                lines[0].color = sf::Color::Black;
                lines[1].position.x = m_player->get<CTransform>().pos.x;
                lines[1].position.y = m_player->get<CTransform>().pos.y;
                lines[1].color = sf::Color::Black;
                m_game.window().draw(lines);
                dot.setPosition(e->get<CFollowPlayer>().home);
                m_game.window().draw(dot);
            }
        }
    }

    /* fps counter */
    float elapsedTime = m_fpsClock.restart().asSeconds();
    float fps = 1.0f / elapsedTime;
    m_fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));
    m_game.window().draw(m_fpsText);

    m_game.window().display();
}

/// @brief helper function for grid drawing; draws line from p1 to p2 on the screen
/// @param p1 first point in line
/// @param p2 second point in line
void Scene_Play::drawLine(const Vec2f &p1, const Vec2f &p2)
{
    sf::Vertex line[] = 
    { 
        sf::Vertex(sf::Vector2f(p1.x, p1.y), sf::Color(255, 255, 255, 50)),
        sf::Vertex(sf::Vector2f(p2.x, p2.y), sf::Color(255, 255, 255, 50))
    };
    m_game.window().draw(line, 2, sf::Lines);
}
