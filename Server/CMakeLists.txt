cmake_minimum_required(VERSION 3.31)
project(Server)

# C++ standard
set(CMAKE_CXX_STANDARD 17)

# debug
set(CMAKE_BUILD_TYPE Debug)

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

# enet
find_library(ENET_LIB enet PATHS /opt/homebrew/lib)

# include directories
include_directories(/opt/homebrew/include)

# link libraries
target_link_libraries(Server PRIVATE ${ENET_LIB})

# print paths for debugging
message(STATUS "ENET Include Dir: ${ENET_INCLUDE_DIR}")
message(STATUS "ENET Library: ${ENET_LIB}")
