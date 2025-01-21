// #include "Timer.hpp"
// #include "SceneLoad.hpp"
// #include "Scene.hpp"
// #include "WorldGenerator.hpp"
// #include "EntityManager.hpp"
// #include "Entity.hpp"

// #include <SFML/Graphics.hpp>

// /// @brief constructs a new SceneLoad object, calls SceneLoad::init
// /// @param gameEngine the game's main engine; required by Scene to access the GameEngine object
// SceneLoad::SceneLoad(GameEngine& gameEngine)
//     : Scene(gameEngine)
// {
//     init();
// }

// /// @brief initialize the LOAD scene
// void SceneLoad::init()
// {
//     m_loadStep = "Loading...";

//     generateWorld();
// }

// /// @brief updates the scene's state
// void SceneLoad::update(std::chrono::duration<long long, std::nano>& lag)
// {
//     sRender();
// }

// /// @brief renders the scene background and text
// void SceneLoad::sRender()
// {
//     // clear the window to a blue
//     m_game.window().setView(m_game.window().getDefaultView());
//     m_game.window().clear(sf::Color(100, 100, 255));

//     // draw the load text
//     m_loadText.setCharacterSize(48);
//     m_loadText.setString(m_loadStep);
//     m_loadText.setFillColor(sf::Color::Black);
//     m_loadText.setPosition({ m_game.window().getSize().x / 2, m_game.window().getSize().y / 2 });
//     m_game.window().draw(m_loadText);

//     /// TODO: draw whatever else I want below

//     m_game.window().display();
// }

// /// @brief quits the game
// void SceneLoad::onEnd()
// {
//     m_game.quit();
// }

// /// @brief randomly generate the playing world
// void SceneLoad::generateWorld()
// {
//     PROFILE_FUNCTION();

//     /// TODO: mountains, caves, lakes, rivers, even terrain, biomes, etc.
//     /// TODO: consider different noise types for speed

//     /*
//         - example:
//         - first pass: top 1/3 dirt, bottom 2/3 stone
//         - second pass: dirt vains in stone and stone blobs in dirt
//         - add in ores (function of depth)
//         - add in caves (prolly function of depth)
//         - add a single deep cave connnecting to surface and going deep at some random location
//         - cut out skyline using noise (Perlin prolly)
//         - add in other things like houses, dungeon, decorations, etc.
//         - grow grass on surface, use code to spread grass and grow vines and things
//         - grow trees
//         - reskin tiles process to make them all fit together nicely
//         - fill in liquids
//         - illumiate everything, pretty it up
//         - use optimization so things aren't insanely slow
//     */

//     // generate world and get tile positions
//     WorldGenerator gen(m_worldMaxCells.x, m_worldMaxCells.y);
//     gen.generateWorld();
//     const std::vector<std::vector<std::string>>& tileMatrix = gen.getTileMatrix();

//     // spawn tiles according to their positions in the grid
//     for (int x = 0; x < m_worldMaxCells.x; ++x)
//     {
//         for (int y = 0; y < m_worldMaxCells.y; ++y)
//         {
//             if (tileMatrix[x][y] == "") continue;

//             Entity tile = m_entityManager.addEntity("tile");
//             tile.addComponent<CAnimation>(m_game.assets().getAnimation(tileMatrix[x][y]), true);
//             tile.addComponent<CTransform>(gridToMidPixel(x, y, tile));
//             tile.addComponent<CBoundingBox>(m_game.assets().getAnimation(tileMatrix[x][y]).getSize());
//             tile.addComponent<CHealth>(tileMatrix[x][y] == "dirt" ? 50 : 80);

//             m_entityManager.addTileToMatrix(tile);
//         }
//     }
// }