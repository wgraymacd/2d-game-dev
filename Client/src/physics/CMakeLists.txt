add_library(Physics INTERFACE) # use interface since only header files

# target_include_directories(Physics INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Physics depends on Utility
target_link_libraries(Physics INTERFACE 
    Utility
)