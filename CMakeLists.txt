cmake_minimum_required(VERSION 3.31)
project(Game)

# C++ standard
set(CMAKE_CXX_STANDARD 17)

# debug
set(CMAKE_BUILD_TYPE Debug)

# executable
add_executable(Game src/main.cpp src/GameEngine.cpp src/Scene_Menu.cpp src/Scene_Play.cpp src/Scene.cpp src/EntityMemoryPool.cpp)

# link SFML libs
find_package(SFML 3.0.0 REQUIRED COMPONENTS Graphics Window System Audio)
target_link_libraries(Game PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)
target_include_directories(Game PRIVATE "/opt/homebrew/Cellar/sfml/2.6.2/include")

# at some point, may want to go modular route: CMakeLists.txt file here, one is src, then one in each directory in source (audio, graphics, etc.) like Step 2 of the CMake tutorial