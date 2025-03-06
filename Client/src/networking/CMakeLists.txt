# collect all source files in this directory
file(GLOB_RECURSE NETWORK_SOURCES "*.cpp")
file(GLOB_RECURSE NETWORK_HEADERS "*.hpp")

# create a library for this module
add_library(Network STATIC
    ${NETWORK_SOURCES}
    ${NETWORK_HEADERS}
)

# this library needs to find headers in this directory
target_include_directories(Network PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR} # /Client/src/networking
    ${ENET_INCLUDE_DIR}
)

# link dependecies
target_link_libraries(Network PUBLIC
    ${ENET_LIB}
    flatbuffers::flatbuffers

    
)