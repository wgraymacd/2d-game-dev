# collect all source files in this directory
file(GLOB_RECURSE CORE_SOURCES "*.cpp")

# create a library for this module
add_library(core STATIC
    ${CORE_SOURCES}
)

# specify where target should look for include files
# target_include_directories(core PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR} # core
#     ${CMAKE_SOURCE_DIR}/src # Client
#     # SFML directory automatically included since package found
# )

# link dependencies
target_link_libraries(core PUBLIC
    network
    physics
    utility
    world

    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)
