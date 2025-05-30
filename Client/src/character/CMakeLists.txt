add_library(character INTERFACE) # use interface since only header files

target_link_libraries(character INTERFACE
    physics
)
