# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(raycaster)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the CMAKE_PREFIX_PATH to include the directory containing the SDL2Config.cmake file
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL2)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Add source files
add_executable(${PROJECT_NAME} main.cpp)

# Add include directories
include_directories(${SDL2_INCLUDE_DIRS})

# Link against SDL2 libraries
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})

link_libraries(SDL2 SDL2main)