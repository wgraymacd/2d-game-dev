cmake_minimum_required(VERSION 3.31)
project(Game)

# C++ standard
set(CMAKE_CXX_STANDARD 17)

# debug
set(CMAKE_BUILD_TYPE Debug)

# executable
add_executable(Game 
    src/main.cpp 
    src/GameEngine.cpp 
    src/SceneMenu.cpp 
    src/ScenePlay.cpp 
    src/Scene.cpp 
    src/EntityMemoryPool.cpp 
    src/SceneLoad.cpp 
    src/Entity.cpp 
    src/Components.cpp
    src/NetworkManager.cpp)

# enet
target_include_directories(Game PRIVATE /opt/homebrew/include/enet)
target_link_libraries(Game PRIVATE /opt/homebrew/lib/libenet.dylib)

# SFML
find_package(SFML 3.0.0 REQUIRED COMPONENTS Graphics Window System Audio)
target_link_libraries(Game PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)

# at some point, may want to go modular route: CMakeLists.txt file here, one is src, then one in each directory in source (audio, graphics, etc.) like Step 2 of the CMake tutorial