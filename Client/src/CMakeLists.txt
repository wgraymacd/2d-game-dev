# add subdirectories
add_subdirectory(core)
add_subdirectory(networking)
add_subdirectory(physics)
add_subdirectory(utility)
add_subdirectory(world)

# create main executable or library
add_executable(${PROJECT_NAME} 
    main.cpp
)

# include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Client/src
)

# link subdirectory libraries and dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE # private makes sense since Client is an executable and not a library
    Core
    Network
    Physics
    Utility
    World
)