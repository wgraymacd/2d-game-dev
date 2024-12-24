#include "GameEngine.h"
#include "Scene_Menu.h"
#include "Assets.hpp"

#include <string>
#include <memory>
#include <SFML/Graphics.hpp>

/// @brief constructs a new GameEngine by calling GameEngine::init
/// @param path the path to the asset configuration file
GameEngine::GameEngine(const std::string &path)
{
    init(path);
}

/// @brief loads assets, creates window, and opens MENU scene
/// @param path the path to the asset configuration file
void GameEngine::init(const std::string &path)
{
    m_assets.loadFromFile(path);
    m_window.create(sf::VideoMode(20 * 64, 20 * 36), "2D Platformer"); // 20 pixel width and height for each block in game, so grid is 64 x 36 cells
    m_window.setFramerateLimit(60);

    addScene("MENU", std::make_shared<Scene_Menu>(*this));
}

/// @brief updates the game state with GameEngine::sUserInput and the active scene's update function
void GameEngine::update()
{
    sUserInput();
    currentScene()->update();
}

/// @brief handles all user input in the game
void GameEngine::sUserInput()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            quit();
        }

        // keyboard clicking
        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
        {
            // if current scene does not have an action associated with key code, skip it
            if (currentScene()->getActionMap().find(event.key.code) == currentScene()->getActionMap().end())
            {
                continue;
            }

            // determine start or end action by whether key was pressed or released
            const std::string actionType = (event.type == sf::Event::KeyPressed) ? "START" : "END";

            // look up the action and send the action to the current scene
            currentScene()->sDoAction(Action(currentScene()->getActionMap().at(event.key.code), actionType));
        }

        // mouse clicking
        if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseButtonReleased)
        {
            // Vec2i mpos(event.mouseButton.x, event.mouseButton.y);

            currentScene()->sDoAction(Action(currentScene()->getActionMap().at(event.mouseButton.button + sf::Keyboard::KeyCount), (event.type == sf::Event::MouseButtonPressed ? "START" : "END"))); // add KeyCount since button codes and key codes overlap
        } 
    }
}

/// @brief continuously calls GameEngine::update while the game is still running
void GameEngine::run()
{
    while (isRunning())
    {
        update();
    }
}

/// @brief quits the game
void GameEngine::quit()
{
    m_running = false;
}

/// @brief gets a boolean representing the game state
/// @return true if game is running, false otherwise
bool GameEngine::isRunning() const
{
    return m_running && m_window.isOpen();
}

/// @brief gets the current scene
/// @return shared pointer to a Scene object
std::shared_ptr<Scene> GameEngine::currentScene()
{
    return m_sceneMap[m_currentScene];
}

/// @brief get the game's window object
/// @return a reference to the sf::RenderWindow object
sf::RenderWindow &GameEngine::window()
{
    return m_window;
}

/// @brief gets all assets stored in the GameEngine object
/// @return Assets object that holds all information related to the game's assets
const Assets &GameEngine::assets() const
{
    return m_assets;
}

/// TODO: implement current scene ending
/// @brief adds a scene to the game
/// @param sceneName the name of the scene to be added (e.g., "MENU")
/// @param scene a new Scene object
/// @param endThisScene a boolean to control whether the current scene ends or not upon moving to the new scene
void GameEngine::addScene(const std::string &sceneName, std::shared_ptr<Scene> scene, bool endThisScene)
{
    m_sceneMap[sceneName] = scene;
    m_currentScene = sceneName;
}

/// @brief change to another active scene
/// @param sceneName name of the scene to change to (e.g., "PLAY")
/// @param endThisScene a boolean to control whether the current scene ends or not upon moving to the new scene
void GameEngine::changeScene(const std::string &sceneName, bool endThisScene)
{
    m_currentScene = sceneName;
}