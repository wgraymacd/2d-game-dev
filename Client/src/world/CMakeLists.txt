add_library(World INTERFACE)

# target_include_directories(World INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# World depends on Physics and Utility
target_link_libraries(World INTERFACE 
    Physics
    Utility
)