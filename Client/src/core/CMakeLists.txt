# collect all source files in this directory
file(GLOB_RECURSE CORE_SOURCES "*.cpp")
file(GLOB_RECURSE CORE_HEADERS "*.hpp")

# create a library for this module
add_library(Core STATIC
    ${CORE_SOURCES} 
    ${CORE_HEADERS}
)

# specify where target should look for include files
target_include_directories(Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src # Client/src
    # SFML directory automatically included since package found
)

# link dependencies
target_link_libraries(Core PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio

    Network
    Physics
    Utility
    World
)