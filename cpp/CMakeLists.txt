cmake_minimum_required(VERSION 3.4.1)
project("Mergen")

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")
add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR=1)

add_library(main SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/aud/commons.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/aud/microphone.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/mem/manifest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mem/queuer.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/rew/rewarder.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/vis/camera.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/vlk/hello_vk.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vlk/compute_vk.cpp
        )

target_link_libraries(main
        android
        camera2ndk
        dl
        log
        m
        mediandk
        OpenSLES
        vulkan
        )