# create main executable or library
add_executable(${PROJECT_NAME} 
    main.cpp
    ServerEngine.cpp
)

# set the output directory
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Server/src
)

# link subdirectory libraries and dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE # private makes sense since Server is an executable and not a library
    ${ENET_LIB}
)
