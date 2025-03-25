cmake_minimum_required(VERSION 3.31)
project(Server)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# debug
set(CMAKE_BUILD_TYPE Debug)

# build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

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

# add subdirectories
add_subdirectory(src)
