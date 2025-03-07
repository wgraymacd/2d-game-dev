cmake_minimum_required(VERSION 3.31)
project(Server)

# C++ standard
set(CMAKE_CXX_STANDARD 17)

# debug
set(CMAKE_BUILD_TYPE Debug)

# executable
add_executable(Server 
    src/main.cpp 
    src/ServerEngine.cpp
    src/NetworkManager.cpp
)

# enet
find_library(ENET_LIB enet REQUIRED PATHS /opt/homebrew/lib) # find the library in one of these directory paths
message(STATUS "ENet:")
message(STATUS "  Library found: ${ENET_LIB}")
get_filename_component(ENET_LIB_DIR ${ENET_LIB} DIRECTORY)
message(STATUS "  Library dir: ${ENET_LIB_DIR}")
if(ENET_LIB)
    find_path(ENET_INCLUDE_DIR enet/enet.h PATHS /opt/homebrew/include)
    message(STATUS "  ENet include dir: ${ENET_INCLUDE_DIR}")
    include_directories(${ENET_INCLUDE_DIR})
endif()

# link libraries
target_link_libraries(Server PRIVATE ${ENET_LIB})
