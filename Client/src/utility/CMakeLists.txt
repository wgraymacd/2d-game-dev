add_library(utility INTERFACE)

target_include_directories(utility INTERFACE
    ### TODO: change this so that Global files are somehow a library or something
    ${PROJECT_SOURCE_DIR}/../../Global
)
