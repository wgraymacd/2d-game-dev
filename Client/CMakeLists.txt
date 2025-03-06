cmake_minimum_required(VERSION 3.31.3)
project(Client VERSION 1.0)

# C++ standard
# TODO: will want to set this based on code for individual libraries, see CMake tutorial step 3
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# dependencies and checks
find_package(SFML 3.0.0 REQUIRED COMPONENTS Graphics Window System Audio)
message(STATUS "SFML:")
message(STATUS "  Found: ${SFML_FOUND}")
message(STATUS "  Version: ${SFML_VERSION}")
get_target_property(SFML_INTERFACE_INCLUDE_DIRS SFML::Graphics INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "  SFML include dirs: ${SFML_INTERFACE_INCLUDE_DIRS}")
if(SFML_FOUND)
    if(TARGET SFML::Graphics)
        message(STATUS "  Using SFML targets")
    else()
        message(STATUS "  Using SFML variables")
        # include_directories(${SFML_INCLUDE_DIR})
    endif()
endif()

find_package(flatbuffers REQUIRED)
message(STATUS "Flatbuffers:")
message(STATUS "  Found: ${flatbuffers_FOUND}")
message(STATUS "  Version: ${flatbuffers_VERSION}")
get_target_property(FLATBUFFERS_INTERFACE_INCLUDE_DIRS flatbuffers::flatbuffers INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "  Flatbuffers include dirs: ${FLATBUFFERS_INTERFACE_INCLUDE_DIRS}")
if(TARGET flatbuffers::flatbuffers)
    message(STATUS "  Target exists: flatbuffers::flatbuffers")
    get_target_property(FLATBUFFERS_TYPE flatbuffers::flatbuffers TYPE)
    message(STATUS "  Target type: ${FLATBUFFERS_TYPE}")
else()
    message(STATUS "  Using flatbuffers variables")
endif()
if(flatbuffers_FOUND)
    if(TARGET flatbuffers::flatbuffers)
        message(STATUS "Using Flatbuffers targets")
  else()
        message(STATUS "Using Flatbuffers variables")
        # include_directories(${FLATBUFFERS_INCLUDE_DIR})
    endif()
endif()

find_library(ENET_LIB enet REQUIRED PATHS /opt/homebrew/lib) # find the library in one of these directory paths
message(STATUS "ENet:")
message(STATUS "  Library found: ${ENET_LIB}")
get_filename_component(ENET_LIB_DIR ${ENET_LIB} DIRECTORY)
message(STATUS "  Library dir: ${ENET_LIB_DIR}")
if(ENET_LIB)
    find_path(ENET_INCLUDE_DIR enet/enet.h PATHS /opt/homebrew/include)
    message(STATUS "  ENet include dir: ${ENET_INCLUDE_DIR}")
    # include_directories(${ENET_INCLUDE_DIR})
endif()

# export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add src directory
add_subdirectory(src)

### test for working includes and linking and such
# # Create a simple executable to verify compilation would work
# add_executable(test_compile test.cpp)

# # Try to link with found libraries
# if(SFML_FOUND)
#     target_link_libraries(test_compile PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)
# endif()
# if(flatbuffers_FOUND AND TARGET flatbuffers::flatbuffers)
#     target_link_libraries(test_compile PRIVATE flatbuffers::flatbuffers)
# elseif(DEFINED flatbuffers_LIBRARIES)
#     target_link_libraries(test_compile PRIVATE ${flatbuffers_LIBRARIES})
# endif()
# if(ENET_LIB)
#     target_link_libraries(test_compile PRIVATE ${ENET_LIB})
# endif()

# # If include directories aren't part of targets, add them explicitly
# if(SFML_FOUND AND NOT TARGET SFML::Graphics)
#     target_include_directories(test_compile PRIVATE ${SFML_INCLUDE_DIR})
# endif()
# if(flatbuffers_FOUND AND NOT TARGET flatbuffers::flatbuffers AND DEFINED FLATBUFFERS_INCLUDE_DIR)
#     target_include_directories(test_compile PRIVATE ${FLATBUFFERS_INCLUDE_DIR})
# endif()

### TODO: go the modular route with CMakeLists.txt files in each subdirectory




### all in one CMakeLists.txt file

# cmake_minimum_required(VERSION 3.31.3)
# project(Client VERSION 1.0)

# # Set C++ standard
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # Find required packages
# find_package(SFML 3.0.0 REQUIRED COMPONENTS Graphics Window System Audio)
# find_package(flatbuffers REQUIRED)
# find_library(ENET_LIB enet REQUIRED PATHS /opt/homebrew/lib)
# find_path(ENET_INCLUDE_DIR enet/enet.h PATHS /opt/homebrew/include)

# # Print diagnostic information
# message(STATUS "SFML found: ${SFML_FOUND}")
# message(STATUS "Flatbuffers found: ${flatbuffers_FOUND}")
# message(STATUS "ENet library: ${ENET_LIB}")
# message(STATUS "ENet include dir: ${ENET_INCLUDE_DIR}")

# # Collect source files from all subdirectories
# file(GLOB_RECURSE CORE_SOURCES "src/core/*.cpp")
# file(GLOB_RECURSE NETWORK_SOURCES "src/network/*.cpp")
# file(GLOB MAIN_SOURCE "src/main.cpp")

# # Define the executable with all source files
# add_executable(${PROJECT_NAME} 
#     ${MAIN_SOURCE}
#     ${CORE_SOURCES} 
#     ${NETWORK_SOURCES} 
# )

# # Export compile commands for IDE integration
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# # Add include directories for all components
# target_include_directories(${PROJECT_NAME} PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/src
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/core
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/network
#     ${ENET_INCLUDE_DIR}
# )

# # Link with all required libraries
# target_link_libraries(${PROJECT_NAME} 
#     PRIVATE SFML::Graphics
#     PRIVATE SFML::Window
#     PRIVATE SFML::System
#     PRIVATE SFML::Audio

#     PRIVATE flatbuffers::flatbuffers
    
#     PRIVATE ${ENET_LIB}
# )