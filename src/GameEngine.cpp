#include "Globals.hpp"
#include "GameEngine.hpp"
#include "SceneMenu.hpp"
#include "Scene.hpp"
#include "Assets.hpp"

#include <string>
#include <memory>
#include <chrono>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

/// TODO: consider multithreading, offload tasks like physics updates and asset loading to keep main game loop responsive
/// TODO: decouple frame rate from updates in main game loop
/// TODO: could introduce a more formal state management system for each scene to handle different modes (e.g., main menu, in-game, paused, game over) and manage transitions between them more gracefully
/// TODO: consider refactoring how the ActionMap is structured. A more efficient lookup mechanism (e.g., a hash map of actions to key/button codes) could improve performance and maintainability
/// TODO: Consider adding a stack of scenes, where you can "push" and "pop" scenes. This would allow for more advanced scenarios like pausing the game (by pushing a pause scene) or implementing menus, without directly replacing scenes
/// TODO: could implement an unordered map (std::unordered_map) for faster lookups. Since scene names are strings, and assuming they are frequently accessed, an unordered map would provide better performance for scene lookups
/// TODO: You might consider implementing a sf::Clock (or similar) to track delta time and simulation speed (m_simulationSpeed). This can ensure consistent updates regardless of the frame rate, allowing for frame-independent movement or animations

/// @brief constructs a new GameEngine by calling GameEngine::init
/// @param path the path to the asset configuration file
GameEngine::GameEngine(const std::string& path)
{
    init(path);
}

/// TODO: make window size dynamic, not hardcoded
/// @brief loads assets, creates window, and opens MENU scene
/// @param path the path to the asset configuration file
void GameEngine::init(const std::string& path)
{
    m_assets.loadFromFile(path);

    // std::ifstream file(path);
    // std::string str;
    // while (file.good())
    // {
    //     file >> str;

    //     if (str == "Window")
    //     {
    //         int width, height, frameRate;
    //         file >> width >> height >> frameRate;
    //         m_window.create(sf::VideoMode({ width, height }), "Game");
    //         m_window.setFramerateLimit(frameRate);
    //     }
    //     else
    //     {
    //         std::cerr << "Unknown asset type: " << str << std::endl;
    //     }
    // }
    // file.close();

    const std::vector<sf::VideoMode>& modes = sf::VideoMode::getFullscreenModes();
    if (modes.empty())
    {
        std::cerr << "No supported fullscreen modes available!" << std::endl;
        exit(-1);
    }
    // sf::VideoMode fullscreenMode = modes[0]; // 4 modes, 1st being highest resolution
    // m_window.create(fullscreenMode, "Game", sf::Style::Default);

    GlobalSettings::windowSize = modes[0].size;
    m_window.create(sf::VideoMode(GlobalSettings::windowSize), "Game", sf::Style::Default); /// TODO: should be a fullscreen option here

    // m_window.create(sf::VideoMode(GlobalSettings::windowSize), "Game");
    m_window.setFramerateLimit(GlobalSettings::frameRate);

    addScene("MENU", std::make_shared<SceneMenu>(*this));
}

/// @brief continuously calls GameEngine::update while the game is still running
void GameEngine::run()
{
    std::chrono::steady_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> lag(0);

    while (isRunning())
    {
        std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<long long, std::nano> elapsedTime = currentTime - lastTime;
        lastTime = currentTime;
        lag += elapsedTime;

        update(lag);
    }
}

/// @brief updates the game state with GameEngine::sUserInput and the active scene's update function
void GameEngine::update(std::chrono::duration<long long, std::nano>& lag)
{
    sUserInput();
    currentScene()->updateState(lag);
}

/// TODO: consider separate functions for keyboard, mouse, controller, touch, etc. to reduce size of this function
/// @brief handles all user input in the game
void GameEngine::sUserInput()
{
    while (const std::optional<sf::Event> event = m_window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            quit();
        }

        if (const sf::Event::Resized* resizedEvent = event->getIf<sf::Event::Resized>())
        {
            // const float originalWidth = static_cast<float>(GlobalSettings::windowSize.x);
            // const float originalHeight = static_cast<float>(GlobalSettings::windowSize.y);
            // const float newWidth = static_cast<float>(resizedEvent->size.x);
            // const float newHeight = static_cast<float>(resizedEvent->size.y);
            // const float originalAspectRatio = originalWidth / originalHeight;
            // const float newAspectRatio = static_cast<float>(newWidth / newHeight);

            // const float scaleFactor = static_cast<float>(newAspectRatio > originalAspectRatio ? newHeight / originalHeight : newWidth / originalWidth);

            // GlobalSettings::windowSize = resizedEvent->size;
            // m_window.setView(sf::View({ 0.0f, 0.0f }, GlobalSettings::windowSize.to<float>() * scaleFactor));

            GlobalSettings::windowSize = resizedEvent->size;
            m_window.setView(sf::View(sf::FloatRect({ 0.0f, 0.0f }, GlobalSettings::windowSize.to<float>())));
            // m_window.setView(sf::View({ 0.0f, 0.0f }, GlobalSettings::windowSize.to<float>()));

            /// TODO: think about performing an action to resize view according to specific scene
        }

        /// keyboard

        if (const sf::Event::KeyPressed* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (auto action = currentScene()->getActionMap().find(static_cast<int>(keyPressed->code));
                action != currentScene()->getActionMap().end())
            {
                currentScene()->sDoAction(Action(action->second, "START"));
            }
        }
        else if (const sf::Event::KeyReleased* keyReleased = event->getIf<sf::Event::KeyReleased>())
        {
            if (auto action = currentScene()->getActionMap().find(static_cast<int>(keyReleased->code));
                action != currentScene()->getActionMap().end())
            {
                currentScene()->sDoAction(Action(action->second, "END"));
            }
        }

        /// mouse

        else if (const sf::Event::MouseButtonPressed* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            currentScene()->sDoAction(Action(
                currentScene()->getActionMap().at(static_cast<int>(mousePressed->button) + sf::Keyboard::KeyCount),
                "START"
            ));
        }
        else if (const sf::Event::MouseButtonReleased* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
        {
            currentScene()->sDoAction(Action(
                currentScene()->getActionMap().at(static_cast<int>(mouseReleased->button) + sf::Keyboard::KeyCount),
                "END"
            ));
        }

        /// TODO: there is now an isKeyPressed event that will tell me if it's being held down, loop through all bound inputs and see if they are being pressed
        // sf::Keyboard::Key key = sf::Keyboard::Key::Space;
        // sf::Mouse::Button button = sf::Mouse::Button::Left;
        // if (sf::Keyboard::isKeyPressed(key))
        // {
        //     if (currentScene()->getActionMap().find(static_cast<int>(key)) == currentScene()->getActionMap().end())
        //     {
        //         continue;
        //     }
        //     else
        //     {
        //         currentScene()->sDoAction(Action(currentScene()->getActionMap().at(static_cast<int>(key)), actionType));
        //     }
        // }

        // handleContinuousInput();
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
sf::RenderWindow& GameEngine::window()
{
    return m_window;
}

/// @brief gets all assets stored in the GameEngine object
/// @return Assets object that holds all information related to the game's assets
Assets& GameEngine::assets()
{
    return m_assets;
}

/// TODO: implement current scene ending
/// @brief adds a scene to the game
/// @param sceneName the name of the scene to be added (e.g., "MENU")
/// @param scene a new Scene object
/// @param endThisScene a boolean to control whether the current scene ends or not upon moving to the new scene
void GameEngine::addScene(const std::string& sceneName, std::shared_ptr<Scene> scene, bool endThisScene)
{
    m_sceneMap[sceneName] = scene;
    m_currentScene = sceneName;
}

/// TODO: implement current scene ending
/// @brief change to another active scene
/// @param sceneName name of the scene to change to (e.g., "PLAY")
/// @param endThisScene a boolean to control whether the current scene ends or not upon moving to the new scene
void GameEngine::changeScene(const std::string& sceneName, bool endThisScene)
{
    m_currentScene = sceneName;
}