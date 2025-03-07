# collect all source files in this directory
file(GLOB_RECURSE NETWORK_SOURCES "*.cpp")
file(GLOB_RECURSE NETWORK_HEADERS "*.hpp")

# create a library for this module
add_library(network STATIC
    ${NETWORK_SOURCES}
    ${NETWORK_HEADERS}
)

# this library needs to find headers in this directory
target_include_directories(network PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR} # /Client/src/networking
    ${CMAKE_SOURCE_DIR}/src
    ${ENET_INCLUDE_DIR}
)

# link dependecies
target_link_libraries(network PUBLIC
    ${ENET_LIB}
    flatbuffers::flatbuffers

    utility
    physics
)