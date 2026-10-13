cmake_minimum_required(VERSION 3.16)

project(opengl_test LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(BUILD_ANIMATION_TESTS "Build the animation regression test" OFF)

if(ANDROID)
    # --- Android: OpenGL ES 1.1 via EGL, driven by android_native_app_glue ---
    add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    target_include_directories(native_app_glue PUBLIC
        ${ANDROID_NDK}/sources/android/native_app_glue
    )

    # Library name must match android:value of the "android.app.lib_name"
    # meta-data entry in AndroidManifest.xml (libgame.so).
    add_library(game SHARED
        game/android_main.cpp
        game/game.cpp
        engine/sound/miniaudio.c
    )

    target_include_directories(game PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/game
        ${CMAKE_CURRENT_SOURCE_DIR}/engine
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/2d
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/input
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/sound
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/platform
    )

    target_link_libraries(game PRIVATE
        native_app_glue
        EGL
        GLESv1_CM
        android
        log
        OpenSLES
    )

    # native_app_glue relies on the linker keeping this symbol even though
    # nothing in our own code calls it directly.
    target_link_options(game PRIVATE "-u" "ANativeActivity_onCreate")
else()
    # --- Desktop: OpenGL compatibility profile via GLEW + GLFW ---
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glfw3 REQUIRED)

    add_executable(opengl_test
        game/game.cpp
        game/example_animated_mesh.cpp
        engine/3d/gltfloader.cpp
        engine/sound/miniaudio.c
    )

    target_include_directories(opengl_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/game
        ${CMAKE_CURRENT_SOURCE_DIR}/engine
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/2d
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/input
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/sound
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/platform
    )

    target_link_libraries(opengl_test PRIVATE
        OpenGL::GL
        GLEW::GLEW
        glfw
    )

    if(UNIX)
        target_link_libraries(opengl_test PRIVATE dl pthread)
    endif()

    if(BUILD_ANIMATION_TESTS)
        enable_testing()
        add_executable(animation_tests
            tests/animation_tests.cpp
            game/example_animated_mesh.cpp
        )
        target_include_directories(animation_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/engine/3d
            ${CMAKE_CURRENT_SOURCE_DIR}/game
        )
        add_test(NAME animation_tests COMMAND animation_tests)
    endif()
endif()

