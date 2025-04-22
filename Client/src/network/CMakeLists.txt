file(GLOB_RECURSE NETWORK_SOURCES "*.cpp")

add_library(network STATIC
    ${NETWORK_SOURCES}
)

target_include_directories(network PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${ENET_INCLUDE_DIR}
)

target_link_libraries(network PUBLIC
    utility
    physics

    ${ENET_LIB}
    # flatbuffers::flatbuffers
)
