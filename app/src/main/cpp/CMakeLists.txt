cmake_minimum_required(VERSION 3.4.1)
project("Mergen")

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")

add_library(main SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/aud/commons.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/aud/engine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/aud/recorder.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/mem/queuer.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/vis/camera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vis/image_reader.cpp
        )
#add_library(vis SHARED) # resulted in an error!

target_link_libraries(main
        android
        camera2ndk
        dl
        log
        m
        mediandk
        OpenSLES)
