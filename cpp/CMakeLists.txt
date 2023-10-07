cmake_minimum_required(VERSION 3.4.1)
project(Mergen VERSION ${VERSION})

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall")
add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR=1)

add_library(main SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        global.h
        manifest.h

        aud/commons.cpp
        aud/commons.h
        aud/microphone.cpp
        aud/microphone.h
        aud/speaker.cpp
        aud/speaker.h

        hpt/touchscreen.cpp
        hpt/touchscreen.h

        mov/vibrator.h

        rew/expression.h
        rew/rewarder.cpp
        rew/rewarder.h

        vis/bitmap_stream.h
        vis/camera.cpp
        vis/camera.h
        vis/colouring.h
        vis/segmentation.cpp
        vis/segmentation.h
        vis/visual_stm.h

        #vlk/hello_vk.cpp
        #vlk/hello_vk.h
        #vlk/global.h
        #vlk/compute_vk.cpp
        #vlk/compute_vk.h
)

# For CLion to index the Android NDK headers
target_include_directories(main PUBLIC
        $ENV{ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include
)

target_link_libraries(main
        android      # for ANativeWindow
        camera2ndk
        log          # for <android/log.h>
        mediandk     # for AImageReader
        OpenSLES
        #vulkan
)
