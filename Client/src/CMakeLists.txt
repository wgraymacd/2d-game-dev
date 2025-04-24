cmake_minimum_required(VERSION 3.10)
project(Client VERSION 1.0)

# TODO: will want to set this based on code for individual libraries, see CMake tutorial step 3
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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
    endif()
endif()

# find_package(flatbuffers REQUIRED)
# message(STATUS "Flatbuffers:")
# message(STATUS "  Found: ${flatbuffers_FOUND}")
# message(STATUS "  Version: ${flatbuffers_VERSION}")
# get_target_property(FLATBUFFERS_INTERFACE_INCLUDE_DIRS flatbuffers::flatbuffers INTERFACE_INCLUDE_DIRECTORIES)
# message(STATUS "  Flatbuffers include dirs: ${FLATBUFFERS_INTERFACE_INCLUDE_DIRS}")
# if(TARGET flatbuffers::flatbuffers)
#     message(STATUS "  Target exists: flatbuffers::flatbuffers")
#     get_target_property(FLATBUFFERS_TYPE flatbuffers::flatbuffers TYPE)
#     message(STATUS "  Target type: ${FLATBUFFERS_TYPE}")
# else()
#     message(STATUS "  Using flatbuffers variables")
# endif()
# if(flatbuffers_FOUND)
#     if(TARGET flatbuffers::flatbuffers)
#         message(STATUS "Using Flatbuffers targets")
#   else()
#         message(STATUS "Using Flatbuffers variables")
#         # include_directories(${FLATBUFFERS_INCLUDE_DIR})
#     endif()
# endif()

find_library(ENET_LIB enet REQUIRED PATHS /opt/homebrew/lib)
message(STATUS "ENet:")
message(STATUS "  Library found: ${ENET_LIB}")
if(ENET_LIB)
    find_path(ENET_INCLUDE_DIR enet/enet.h PATHS /opt/homebrew/include)
    message(STATUS "  ENet include dir: ${ENET_INCLUDE_DIR}")
endif()

add_compile_options(
    -std=c++23                       # Specify C++ standard

    # -fcolor-diagnostics              # Enables colored output in terminal for compiler warnings and errors
    # -fansi-escape-codes              # Enables ANSI escape sequences in compiler output for better formatting

    -g                               # Generates debug information
    -O0                              # No optimization

    -Wall                            # Enable all common warnings
    -Wextra                          # Enable even more warnings
    -Wpedantic                       # Issue warnings for strict standard compliance
    -Werror                          # Treat all warnings as errors

    -Wconversion                     # Warn on implicit conversions that may alter a value
        # -Wno-sign-conversion

    -Wshadow                         # Warn when a variable shadows another variable
    -Wunreachable-code               # Warn if the compiler detects unreachable code
    -Wcast-align                     # Warn if a pointer cast may result in misaligned access
    -Wcast-qual                      # Warn if a pointer cast discards qualifiers
    -Wnull-dereference               # Warn if a null pointer dereference is detected
    -Wimplicit-fallthrough           # Warn if a switch statement falls through without a case
    -Wstrict-aliasing                # Warn if a pointer type is aliased to a different type

    # -fsanitize=address               # Enable AddressSanitizer
    # -fsanitize=undefined             # Enable UndefinedBehaviorSanitizer
    # -fno-omit-frame-pointer          # Required for sanitizers to provide accurate stack traces
)

add_subdirectory(core)
add_subdirectory(network)
add_subdirectory(physics)
add_subdirectory(utility)
add_subdirectory(world)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    core
    network
    physics
    utility
    world
)
