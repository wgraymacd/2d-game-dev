cmake_minimum_required(VERSION 3.31)
project(Game)

# C++ standard
set(CMAKE_CXX_STANDARD 17)

# executable
add_executable(Game src/main.cpp src/GameEngine.cpp src/Scene_Menu.cpp src/Scene_Play.cpp src/Scene.cpp)

# link SFML libs
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
target_link_libraries(Game PRIVATE sfml-graphics sfml-window sfml-system)
target_include_directories(Game PRIVATE "/opt/homebrew/Cellar/sfml/2.6.2/include")

# at some point, may want to go modular route: CMakeLists.txt file here, one is src, then one in each directory in source (audio, graphics, etc.) like Step 2 of the CMake tutorial