cmake_minimum_required(VERSION 3.16)

project(opengl_test LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(test
    game.cpp
    engine/sound/miniaudio.c
)

target_include_directories(test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/2d
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/input
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/sound
)

target_link_libraries(test PRIVATE
    OpenGL::GL
    GLEW::GLEW
    glfw
)

if(UNIX)
    target_link_libraries(test PRIVATE dl pthread)
endif()
