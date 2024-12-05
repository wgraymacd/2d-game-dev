#include "GameEngine.h"
#include "Assets.hpp"
#include "Scene_Play.h"
#include "Scene_Menu.h"

#include <iostream>

GameEngine::GameEngine(const std::string& path)
{
    init(path);
}

// initializer loads assets, creates window, and opens MENU scene
void GameEngine::init(const std::string& path)
{
    m_assets.loadFromFile(path);

    m_window.create(sf::VideoMode(40*32, 40*18), "2D Platformer"); // 40 pixel width and height for each block in game, so grid is 48 x 27 cells
    m_window.setFramerateLimit(60);

    // go to menu screen
    addScene("MENU", std::make_shared<Scene_Menu>(*this));
}

// update the game state
void GameEngine::update()
{
	sUserInput();
	currentScene()->update();
}

// user input system
void GameEngine::sUserInput()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            quit();
        }

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
    }
}

// run (continuously call update()) the game
void GameEngine::run()
{
    while (isRunning())
    {
        update();
    }
}

// quit the game (stop running)
void GameEngine::quit()
{
	m_running = false;
}

// get a bool representing the game state
bool GameEngine::isRunning() const
{
    return m_running && m_window.isOpen();
}

// get the current scene
std::shared_ptr<Scene> GameEngine::currentScene()
{
    return m_sceneMap[m_currentScene];
}

// return a reference to the window object
sf::RenderWindow& GameEngine::window()
{
    return m_window;
}

// return game engine assets
const Assets& GameEngine::assets() const
{
	return m_assets;
}

void GameEngine::addScene(const std::string& sceneName, std::shared_ptr<Scene> scene, bool endThisScene)
{
    m_sceneMap[sceneName] = scene;
	m_currentScene = sceneName;
}

void GameEngine::changeScene(const std::string& sceneName, bool endThisScene)
{
	m_currentScene = sceneName;
}