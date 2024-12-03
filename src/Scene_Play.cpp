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
    registerAction(sf::Keyboard::T, "TOGLLE_TEXTURE");
    registerAction(sf::Keyboard::C, "TOGLLE_COLLISION");
    registerAction(sf::Keyboard::G, "TOGLLE_GRID");

    // player
    registerAction(sf::Keyboard::W, "JUMP");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::J, "SHOOT");

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game.assets().getFont("PixelCowboy"));

    loadLevel(levelPath);
}

Vec2f Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
    // TODO: this func takes in a grid (x, y) pos and an entity
    // return Vec2f indicating where the center pos of the entity should be
    // use the entity's animation size to position it correctly
    // the size of the grid width and height is stored in m_gridSize
    // the bottom left corner of the animation should align with the bottom left corner of the grid cell
    // remember that SFML has (0, 0) in top left

    float xPos = gridX * m_gridSize.x + entity->get<CAnimation>().animation.getSize().x / 2;
    float yPos = m_game.window().getSize().y - gridY * m_gridSize.y - entity->get<CAnimation>().animation.getSize().y / 2;

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
        std::cout << "type: " << type << std::endl;
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

            std::cout << "added Tile: " << tile->get<CAnimation>().animation.getName() << " " << tile->get<CTransform>().pos.x << " " << tile->get<CTransform>().pos.y << std::endl;
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

    std::cout << "Spawnign player" << std::endl;
    spawnPlayer();
}

void Scene_Play::spawnPlayer()
{
    if (!m_entityManager.getEntities("player").empty())
    {
        m_entityManager.getEntities("player").front()->destroy();
    }

    std::shared_ptr<Entity> player = m_entityManager.addEntity("player");
    player->add<CAnimation>(m_game.assets().getAnimation("Ground"), true);
    player->add<CTransform>(gridToMidPixel(m_playerConfig.GX, m_playerConfig.GY, player));
    player->add<CBoundingBox>(Vec2f(m_playerConfig.CW, m_playerConfig.CH));
    player->add<CState>("stand");
    player->add<CInput>();
    player->add<CGravity>(m_playerConfig.GRAVITY);
}

// spawn a bullet at the location of entity in the direction the entity is facing
void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
    // spawn bullet
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
    // TODO: player movement based on CInput, gravity, max speed in x and y
    // note: use scale.x = -1 to flip entity direction

    std::string &state = player()->get<CState>().state;
    CInput &input = player()->get<CInput>();
    CTransform &trans = player()->get<CTransform>();

    Vec2f velToAdd(0, 0);

    if (input.right)
    {
        if (abs(trans.velocity.x) <= m_playerConfig.SM)
        {
            velToAdd.x += m_playerConfig.SX;
        }
        state = "run";
        trans.scale.x = abs(trans.scale.x);
    }
    if (input.left)
    {
        if (abs(trans.velocity.x) <= m_playerConfig.SM)
        {
            velToAdd.x -= m_playerConfig.SX;
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

    velToAdd.y += m_playerConfig.GRAVITY;
    trans.velocity += velToAdd;
    trans.pos += trans.velocity;
}

void Scene_Play::sLifespan()
{
    // TODO: check lifespan of entities that have time, and destroy them if they go over
}

void Scene_Play::sCollision()
{
    // recall SFML (0, 0) in top left

    // TODO: implement Physics::GetOverlap()
    // implement bullet/tile, player tile (remember to set CState for the animation system to use)
    // check if player has fallen in hole
    // don't let player walk off left side of map
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
        player()->add<CAnimation>(m_game.assets().getAnimation("Ground"), true);
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

    // draw all entity collision bounding boxed with a rectangle
    if (m_drawCollision)
    {
        for (auto e : m_entityManager.getEntities())
        {
            if (e->has<CBoundingBox>())
            {
                auto &box = e->get<CBoundingBox>();
                auto &transform = e->get<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box.size)); // again, may have to use x and y components as separate arguments for these lines 
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
    if (m_drawGrid)
    {
        // float leftX = m_game.window().getView().getCenter().x - /* more here */;
        // float rightX = leftX + width() + m_gridSize.x;
        // float nextGridX = leftX - ((int)leftX % (int)m_gridSize./* more here */);

        // for (float x = nextGridX; x < rightX; x += m_gridSize.x)
        // {
        //     drawLine(Vec2f(x, 0), Vec2f(x, height()));
        // }

        // for (float y = 0; y < height(); y += m_gridSize.y)
        // {
        //     drawLine(Vec2f(leftX, height() - y), Vec2f(rightX, h /* more here */));

        //     for (float x = nextGridX; x < rightX; x += m_gridSize /* may be more here */)
        //     {
        //         std::string xCell = std::to_string((int)x / (int)/* more here */);
        //         std::string yCell = std::to_string((int)y / (int)/* more here */);
        //         m_gridText.setString("(" + xCell + "," + yCell + /* more here */);
        //         m_gridText.setPosition(x + 3, height() - y - m_gridSize /* possible more here */);
        //         m_game.window().draw(m_gridText);
        //     }
        // }
    }

    m_game.window().display();
}

void Scene_Play::drawLine(const Vec2f &p1, const Vec2f &p2)
{
    // sf::Vertex line[] = {sf::Vector(p1.x, p1.y), sf::Vector2f /* more here */};
    // m_game.window().draw(line, 2, sf::Lines);
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
