# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)
project(raycaster)

# Add the executable target
add_executable(raycaster main.cpp)

# Set the root directory for the libraries
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs)

# Include the SDL2 and SDL2_ttf headers
include_directories(${LIBS_DIR}/SDL2/include)
include_directories(${LIBS_DIR}/SDL2_ttf/include)

# Specify the path to the libraries for 64-bit
link_directories(${LIBS_DIR}/SDL2/lib/x64)
link_directories(${LIBS_DIR}/SDL2_ttf/lib/x64)

# Find the SDL2 and SDL2_ttf libraries
find_library(SDL2_LIB
    NAMES SDL2
    HINTS ${LIBS_DIR}/SDL2/lib/x64
)

find_library(SDL2MAIN_LIB
    NAMES SDL2main
    HINTS ${LIBS_DIR}/SDL2/lib/x64
)

find_library(SDL2_TTF_LIB
    NAMES SDL2_ttf
    HINTS ${LIBS_DIR}/SDL2_ttf/lib/x64
)

# Verify SDL2 and SDL2main library paths
message(STATUS "SDL2_LIB found: ${SDL2_LIB}")
message(STATUS "SDL2MAIN_LIB found: ${SDL2MAIN_LIB}")
message(STATUS "SDL2_TTF_LIB found: ${SDL2_TTF_LIB}")

# Link the libraries to your target
target_link_libraries(raycaster PRIVATE ${SDL2_LIB} ${SDL2MAIN_LIB} ${SDL2_TTF_LIB})

# Ensure the correct linker flags are passed for SDL2
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dmain=SDL_main")