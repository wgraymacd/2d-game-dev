add_library(physics INTERFACE) # use interface since only header files

target_link_libraries(physics INTERFACE
    utility
)
