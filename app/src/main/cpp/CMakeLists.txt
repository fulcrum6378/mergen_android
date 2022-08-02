cmake_minimum_required(VERSION 3.22.1)
project("mergen")

set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -Werror")
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
add_library(main SHARED main.cpp)

target_include_directories(main PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue)
target_link_libraries(main
        android
        log
        native_app_glue)
