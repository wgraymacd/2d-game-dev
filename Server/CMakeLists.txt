cmake_minimum_required(VERSION 3.31)
project(Server)

# C++ standard
set(CMAKE_CXX_STANDARD 17)

# debug
set(CMAKE_BUILD_TYPE Debug)

# enet library paths (for Homebrew on macOS)
find_path(ENET_INCLUDE_DIR 
    NAMES enet/enet.h
    PATHS 
    /opt/homebrew/include
)

find_library(ENET_LIB 
    NAMES enet
    PATHS 
    /opt/homebrew/lib
)

# executable
add_executable(Server 
    src/main.cpp 
    src/GameEngine.cpp 
    src/SceneMenu.cpp 
    src/ScenePlay.cpp 
    src/Scene.cpp 
    src/EntityMemoryPool.cpp 
    src/SceneLoad.cpp 
    src/Entity.cpp 
    src/Components.cpp
    src/NetworkManager.cpp
)

# include directories
include_directories(${ENET_INCLUDE_DIR})

# link libraries
target_link_libraries(Server PRIVATE ${ENET_LIB})

# print paths for debugging
message(STATUS "ENET Include Dir: ${ENET_INCLUDE_DIR}")
message(STATUS "ENET Library: ${ENET_LIB}")

# at some point, may want to go modular route: CMakeLists.txt file here, one is src, then one in each directory in source (audio, graphics, etc.) like Step 2 of the CMake tutorial