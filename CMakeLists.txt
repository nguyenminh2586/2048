cmake_minimum_required(VERSION 3.10)
project(2048_Game)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 packages
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} include)

# Add source files
file(GLOB SOURCES "src/*.cpp")

# Create executable
add_executable(2048_Game ${SOURCES})

# Link libraries
target_link_libraries(2048_Game ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_MIXER_LIBRARIES})

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
