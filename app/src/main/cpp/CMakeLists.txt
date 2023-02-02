cmake_minimum_required(VERSION 3.4.1)
project("Mergen")

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")
add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR=1)

add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

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
target_include_directories(main PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue)

target_link_libraries(main
        android
        camera2ndk
        dl
        log
        m
        mediandk
        native_app_glue
        OpenSLES
        vulkan
        )
