cmake_minimum_required(VERSION 3.4.1)
project("Mergen")

# Include the GameActivity static lib to the project.
find_package(game-activity REQUIRED CONFIG)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u \
    Java_com_google_androidgamesdk_GameActivity_initializeNativeCode")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_VERBOSE_MAKEFILE on)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")
add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR=1)

add_library(main SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/aud/commons.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/aud/engine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/aud/recorder.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/mem/queuer.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/vis/camera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vis/image_reader.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_test.cpp
        )

target_link_libraries(main
        android
        camera2ndk
        dl
        log
        m
        mediandk
        game-activity::game-activity_static
        OpenSLES
        vulkan
        )
