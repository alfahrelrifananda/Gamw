cmake_minimum_required(VERSION 3.10)
project(Gamw LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to our Find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find SDL2 and SDL2_ttf
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Create executable — list your source files explicitly or use GLOB
file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# === Modern imported targets (preferred) ===
if(TARGET SDL2::SDL2)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf)
else()
    # Fallback for distros that don't create SDL2::SDL2 (Ubuntu/Debian)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})
endif()

# Include directory for your own headers (Game.h, etc.)
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Copy assets
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
