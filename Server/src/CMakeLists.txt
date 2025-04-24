cmake_minimum_required(VERSION 3.10)
project(Server VERSION 1.0)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
# Find enet library
find_library(ENET_LIB enet REQUIRED PATHS /opt/homebrew/lib)
message(STATUS "ENet:")
message(STATUS "  Library found: ${ENET_LIB}")
get_filename_component(ENET_LIB_DIR ${ENET_LIB} DIRECTORY)
message(STATUS "  Library dir: ${ENET_LIB_DIR}")
if(ENET_LIB)
    find_path(ENET_INCLUDE_DIR enet/enet.h PATHS /opt/homebrew/include)
    message(STATUS "  ENet include dir: ${ENET_INCLUDE_DIR}")
endif()

# Add compiler options
# Can use target compile options instead of add to apply these to specific parts only
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

# Generate compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# create main executable
add_executable(${PROJECT_NAME}
    main.cpp
    ServerEngine.cpp
)

# set the output directory
# set_target_properties(${PROJECT_NAME} PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
# )

# link libraries and dependencies to Server target
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${ENET_LIB}
)

# include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../../Global # Global files shared by Server and Client
    ${ENET_INCLUDE_DIR}
)
