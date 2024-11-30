#include "Scene_Play.h"
#include "Physics.hpp"
#include "Assets.hpp"
#include "GameEngine.h"
#include "Components.hpp"
#include "Action.hpp"

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

    // TODO: register all other gameplay actions

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game.assets().getFont("Tech"));

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

    return Vec2f(0, 0);
}

void Scene_Play::loadLevel(const std::string &filename)
{
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    // TODO: read in the level file and add the appropriate entities
    // use the PlayerConfig struct m_playerConfig to store player properties
    // this struct is defined at the top of Scene_Play.h

    // note: all code below is sample code which shows how to set up, it should be removed

    spawnPlayer();

    // some sample entities
    auto brick = m_entityManager.addEntity("tile");

    // always add CAnimation component first so that gridToMidPixel works
    brick->add<CAnimation>(m_game.assets().getAnimation("Brick"), true);
    brick->add<CTransform>(Vec2f(96, 480));

    // note: final code should position entity with the grid position read from config file
    // brick->add<CTransform>(gridToMidPixel(gridX, gridY, brick));

    if (brick->get<CAnimation>().animation.getName() == "Brick")
    {
        // good way of identifying if a tile is a brick
    }

    auto block = m_entityManager.addEntity("tile");
    block->add<CAnimation>(m_game.assets().getAnimation("Block"), true);
    block->add<CTransform>(Vec2f(224, 480));

    // add a bounding box, this will now show up if we press the 'c' key
    block->add<CBoundingBox>(m_game.assets().getAnimation("Block").getSize());

    auto question = m_entityManager.addEntity("tile");
    question->add<CAnimation>(m_game.assets().getAnimation("Question"), true);
    question->add<CTransform>(Vec2f(352, 480));

    // note: example
    // components are now returned as references rather than pointers
    // if you don't specify a reference variable type, it will copy the component
    // example: auto transform1 = entity->get<CTransform>()
    // must use auto& transform2 = entity->get<CTransform>()
}

void Scene_Play::spawnPlayer()
{
    // here is a sample player entity to use to construct other ones
    auto player = m_entityManager.addEntity("player");
    player->add<CAnimation>(m_game.assets().getAnimation("Stand"), true);
    player->add<CTransform>(Vec2f(224, 352));
    player->add<CBoundingBox>(Vec2f(48, 48));
    player->add<CState>("stand");
    player->add<CInput>();

    // TODO: add remaining components to player
    // destory dead player if respawning
}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
    // spawn bullet
}

void Scene_Play::update()
{
    m_entityManager.update();

    // TODO: implement pause functionality

    sMovement();
    sLifespan();
    sCollision();
    sAnimation();
    sGUI();
    sRender();
}

void Scene_Play::sMovement()
{
    // TODO: player movement based on CInput, gravity, max speed in x and y
    // note: use scale.x = -1 to flip entity direction
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
    }
    else if (action.type() == "END")
    {
        // fill in here for other action (and above too)
    }
}

void Scene_Play::sAnimation()
{
    // TODO: Complete the Animation class code first
    // for each entity with an animation, call entity->get<CAnimation>().animation.update()
    // if animation is not repeated, and it has ended, destroy the entity

    // set animation of player based on its CState component
    // here's an example
    if (player()->get<CState>().state == "run")
    {
        // change its animation to a repeating run animation
        // note: adding a component that already exists simple overwrites it
        player()->add<CAnimation>(m_game.assets().getAnimation("Run"), true);
    }
}

void Scene_Play::onEnd()
{
    // TODO: when scene ends, change back to menu scene
    // use m_game.changeScene(correct params);
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
        m_game.window().clear(sf::Color(100, 100, 255));
    }
    else
    {
        m_game.window().clear(sf::Color(50, 50, 150));
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
        for (auto e : m_entityManager.getEntities()) // get entities returns reference
        {
            if (e->has<CAnimation>())
            {
                auto &transform = e->get<CTransform>();
                auto &animation = e->get<CAnimation>().animation;
                animation.getSprite().setRotation(transform.rotAngle);
                animation.getSprite().setPosition(transform.pos); // may have to use pos.x and pos.y as two arguments, not sure
                animation.getSprite().setScale(transform.scale); // same as above

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
