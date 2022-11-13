cmake_minimum_required(VERSION 3.4.1)
project("Mergen")

set(CMAKE_VERBOSE_MAKEFILE on)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")

add_library(main SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/android_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vis/camera_engine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vis/camera_listeners.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vis/camera_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vis/camera_utils.cpp)
#add_library(vis SHARED) # resulted in an error!

target_link_libraries(main

        android
        camera2ndk
        OpenSLES
        dl
        log
        m)
