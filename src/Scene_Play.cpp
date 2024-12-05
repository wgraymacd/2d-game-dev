#include "Scene_Play.h"
#include "Physics.hpp"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"

#include <fstream>

Scene_Play::Scene_Play(GameEngine &gameEngine, const std::string &levelPath)
    : Scene(gameEngine), m_levelPath(levelPath)
{
    init(levelPath);
}

void Scene_Play::init(const std::string &levelPath)
{
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
    registerAction(sf::Keyboard::G, "TOGGLE_GRID");

    // player
    registerAction(sf::Keyboard::W, "JUMP");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::J, "SHOOT");

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game.assets().getFont("PixelCowboy"));

    loadLevel(levelPath);
}

// return Vec2f indicating where the center pos of the entity should be
Vec2f Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
    float xPos = gridX * m_gridSize.x + entity->get<CAnimation>().animation.getSize().x / 2.0f;
    float yPos = m_game.window().getSize().y - gridY * m_gridSize.y - entity->get<CAnimation>().animation.getSize().y / 2.0f;

    return Vec2f(xPos, yPos);
}

void Scene_Play::loadLevel(const std::string &filename)
{
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    // read in the level file and add the appropriate entities
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Level file could not be opened: " << filename << std::endl;
        exit(-1);
    }

    std::string type;

    while (file >> type)
    {
        if (type == "Tile")
        {
            std::shared_ptr<Entity> tile = m_entityManager.addEntity("tile");

            // always add CAnimation component first so that gridToMidPixel works
            std::string animation;
            file >> animation;
            tile->add<CAnimation>(m_game.assets().getAnimation(animation), true);

            float gridX, gridY;
            file >> gridX >> gridY;
            tile->add<CTransform>(gridToMidPixel(gridX, gridY, tile));

            tile->add<CBoundingBox>(m_game.assets().getAnimation(animation).getSize());

            std::cout << "added Tile: " << tile->id() << " " << tile->get<CAnimation>().animation.getName() << " " << tile->get<CTransform>().pos.x << " " << tile->get<CTransform>().pos.y << std::endl;
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

void Scene_Play::spawnPlayer()
{
    if (!m_entityManager.getEntities("player").empty())
    {
        m_entityManager.getEntities("player").front()->destroy();
    }

    std::shared_ptr<Entity> player = m_entityManager.addEntity("player");
    player->add<CAnimation>(m_game.assets().getAnimation("GroundBlack"), true);
    player->add<CTransform>(gridToMidPixel(m_playerConfig.GX, m_playerConfig.GY, player));
    player->add<CBoundingBox>(Vec2f(m_playerConfig.CW, m_playerConfig.CH));
    player->add<CState>("stand");
    player->add<CInput>();
    player->add<CGravity>(m_playerConfig.GRAVITY);
}

// spawn a bullet at the location of entity in the direction the entity is facing
void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
    std::shared_ptr<Entity> bullet = m_entityManager.addEntity("bullet");
    bullet->add<CAnimation>(m_game.assets().getAnimation(m_playerConfig.BA), true);
    bullet->add<CTransform>(entity->get<CTransform>().pos, Vec2f(entity->get<CTransform>().scale.x * 15.0f, 0.0f), Vec2f(1.0f, 1.0f), 0.0f);
    bullet->add<CBoundingBox>(bullet->get<CAnimation>().animation.getSize());
    bullet->add<CLifespan>(60, m_currentFrame); // TODO: Lifespan component could use some cleanup, along with everything in general once finished with functionality (didn't end up using everything the way Dave set it up)
}

void Scene_Play::update()
{
    if (!m_paused)
    {
        m_entityManager.update();

        sMovement();
        sLifespan();
        sCollision();
        sAnimation();
    }

    sGUI();
    sRender();
}

void Scene_Play::sMovement()
{
    // player 

    std::string &state = player()->get<CState>().state;
    CInput &input = player()->get<CInput>();
    CTransform &trans = player()->get<CTransform>();

    Vec2f velToAdd(0, 0);

    velToAdd.y += m_playerConfig.GRAVITY;

    if (!input.left && !input.right)
    {
        // float slowing = m_playerConfig.SX / 2;
        float slowing = 1;
        if (abs(trans.velocity.x) >= slowing)
        {
            velToAdd.x += (trans.velocity.x > 0 ? -slowing : slowing);
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
        state = "run";
        trans.scale.x = abs(trans.scale.x);
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
        state = "run";
        trans.scale.x = -abs(trans.scale.x);
    }

    if (input.up && input.canJump)
    {
        velToAdd.y -= m_playerConfig.SY;
        state = "jump";
        input.canJump = false; // set to true in sCollision (must see if on the ground)
    }

    // on release of jump key
    if (!input.up && trans.velocity.y < 0)
    {
        trans.velocity.y = 0;
    }

    trans.velocity += velToAdd;
    trans.prevPos = trans.pos;
    trans.pos += trans.velocity;

    if (trans.velocity.x == 0 && trans.velocity.y == 0)
    {
        if (input.down)
        {
            state = "crouch";
        }
        else
        {
            state = "stand";
        }
    }

    // bullets
    for (auto &bullet : m_entityManager.getEntities("bullet"))
    {
        bullet->get<CTransform>().pos += bullet->get<CTransform>().velocity;
    }
    if (input.shoot && input.canShoot)
    {
        spawnBullet(player());
    }
}

// TODO: not working, fix
void Scene_Play::sLifespan()
{
    for (auto &e : m_entityManager.getEntities())
    {
        if (e->has<CLifespan>())
        {
            int lifespan = e->get<CLifespan>().lifespan;

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
}

void Scene_Play::sCollision()
{
    CTransform &trans = player()->get<CTransform>();

    for (auto &tile : m_entityManager.getEntities("tile"))
    {
        // player and tiles

        Vec2f overlap = Physics::GetOverlap(player(), tile);
        Vec2f prevOverlap = Physics::GetPreviousOverlap(player(), tile);

        // collision
        if (overlap.y > 0 && overlap.x > 0)
        {
            // we are colliding in y-direction this frame since previous frame already had x-direction overlap
            if (prevOverlap.x > 0)
            {
                // player moving down
                if (trans.velocity.y > 0)
                {
                    trans.pos.y -= overlap.y;
                    player()->get<CState>().state = "stand";
                    player()->get<CInput>().canJump = true; // set to false after jumping in sMovement()
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
                std::cout << "tile " << tile->id() << "(" << tile->get<CTransform>().pos.x << ", " << tile->get<CTransform>().pos.y << ")" << " and bullet " << bullet->id() << "(" << bullet->get<CTransform>().pos.x << ", " << bullet->get<CTransform>().pos.y << ")" << " collided" << std::endl;
                // TODO: do whatever else here I might want (animations, tile weakening, etc.)
                bullet->destroy();
            }
        }
    }

    // player falls below map
    if (trans.pos.y - player()->get<CAnimation>().animation.getSize().y / 2 > m_game.window().getSize().y)
    {
        spawnPlayer();
    }

    // side of map
    Animation &anim = player()->get<CAnimation>().animation;
    if (trans.pos.x < anim.getSize().x / 2)
    {
        trans.pos.x = anim.getSize().x / 2;
        trans.velocity.x = 0;
    }
}

// set CInput variables accordingly, no action logic here
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
            player()->get<CInput>().up = true;
        }
        else if (action.name() == "LEFT")
        {
            player()->get<CInput>().left = true;
        }
        else if (action.name() == "RIGHT")
        {
            player()->get<CInput>().right = true;
        }
        else if (action.name() == "SHOOT")
        {
            player()->get<CInput>().shoot = true;
        }
    }
    else if (action.type() == "END")
    {
        if (action.name() == "JUMP")
        {
            player()->get<CInput>().up = false;
        }
        else if (action.name() == "LEFT")
        {
            player()->get<CInput>().left = false;
        }
        else if (action.name() == "RIGHT")
        {
            player()->get<CInput>().right = false;
        }
        else if (action.name() == "SHOOT")
        {
            player()->get<CInput>().shoot = false;
        }
    }
}

void Scene_Play::sAnimation()
{
    // TODO: Complete the Animation class code first
    // for each entity with an animation, call entity->get<CAnimation>().animation.update()
    // if animation is not repeated, and it has ended, destroy the entity

    // set animation of player based on its CState component
    // here's an example
    if (player()->get<CState>().state == "stand")
    {
        // change its animation to a repeating run animation
        // note: adding a component that already exists simple overwrites it
        player()->add<CAnimation>(m_game.assets().getAnimation("GroundBlack"), true);
    }
}

void Scene_Play::onEnd()
{
    m_game.changeScene("MENU");
}

void Scene_Play::sGUI()
{
    // some GUI if I want
}

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

    // set the viewport of the window to be centered on the player if player is not on left bound of world
    // auto &pPos = player()->get<CTransform>().pos;
    // float windowCenterX = std::max(m_game.window().getSize().x / /* more here */);
    // sf::View view = m_game.window().getView();
    // view.setCenter(windowCenterX, m_game.window().getSize().y - /* more here */);
    // m_game.window().setView(view);

    // draw all entity textures / animations
    if (m_drawTextures)
    {
        for (auto e : m_entityManager.getEntities()) // getEntities() returns reference
        {
            if (e->has<CAnimation>())
            {
                auto &transform = e->get<CTransform>();
                auto &animation = e->get<CAnimation>().animation;
                animation.getSprite().setRotation(transform.rotAngle);
                animation.getSprite().setPosition(transform.pos);
                animation.getSprite().setScale(transform.scale);

                m_game.window().draw(animation.getSprite());
            }
        }
    }

    // draw all entity collision bounding boxes with a rectangle
    if (m_drawCollision)
    {
        for (auto e : m_entityManager.getEntities())
        {
            if (e->has<CBoundingBox>())
            {
                auto &box = e->get<CBoundingBox>();
                auto &transform = e->get<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box.size));
                rect.setOrigin(sf::Vector2f(box.halfSize));
                rect.setPosition(transform.pos);
                rect.setFillColor(sf::Color(0, 0, 0, 0));
                rect.setOutlineColor(sf::Color(255, 255, 255, 255));
                rect.setOutlineThickness(1);
                m_game.window().draw(rect);
            }
        }
    }

    // draw the grid
    // if (m_drawGrid)
    // {
    //     float leftX = m_game.window().getView().getCenter().x - /* more here */;
    //     float rightX = leftX + width() + m_gridSize.x;
    //     float nextGridX = leftX - ((int)leftX % (int)m_gridSize./* more here */);

    //     for (float x = nextGridX; x < rightX; x += m_gridSize.x)
    //     {
    //         drawLine(Vec2f(x, 0), Vec2f(x, height()));
    //     }

    //     for (float y = 0; y < height(); y += m_gridSize.y)
    //     {
    //         drawLine(Vec2f(leftX, height() - y), Vec2f(rightX, h /* more here */));

    //         for (float x = nextGridX; x < rightX; x += m_gridSize /* may be more here */)
    //         {
    //             std::string xCell = std::to_string((int)x / (int)/* more here */);
    //             std::string yCell = std::to_string((int)y / (int)/* more here */);
    //             m_gridText.setString("(" + xCell + "," + yCell + /* more here */);
    //             m_gridText.setPosition(x + 3, height() - y - m_gridSize /* possible more here */);
    //             m_game.window().draw(m_gridText);
    //         }
    //     }
    // }

    m_game.window().display();
}

void Scene_Play::drawLine(const Vec2f &p1, const Vec2f &p2)
{
    sf::Vertex line[] = { sf::Vector2f(p1.x, p1.y), sf::Vector2f(p2.x, p2.y) };
    m_game.window().draw(line, 2, sf::Lines);
}

// helper function to get player easily
std::shared_ptr<Entity> Scene_Play::player()
{
    auto &players = m_entityManager.getEntities("player");

    if (!players.empty())
    {
        return players.front();
    }
    else
    {
        std::cerr << "No active players" << std::endl;
        exit(-1);
    }
}
