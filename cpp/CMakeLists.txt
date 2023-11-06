cmake_minimum_required(VERSION 3.4.1)
project(Mergen VERSION ${VERSION})

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall")
add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR=1)

add_library(main SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        global.hpp
        manifest.hpp

        aud/commons.hpp
        aud/microphone.cpp
        aud/microphone.hpp
        aud/speaker.cpp
        aud/speaker.hpp

        hpt/touchscreen.cpp
        hpt/touchscreen.hpp

        mov/vibrator.hpp

        rew/expression.hpp
        rew/rewarder.cpp
        rew/rewarder.hpp

        scm/perception.hpp

        vis/bitmap_stream.hpp
        vis/camera.cpp
        vis/camera.hpp
        vis/colouring.hpp
        vis/memory.cpp
        vis/memory.hpp
        vis/segment.hpp
        vis/segmentation.cpp
        vis/segmentation.hpp
        vis/visual_stm.cpp
        vis/visual_stm.hpp

        #vlk/compute_vk.cpp
        #vlk/compute_vk.h
        #vlk/global.h
        #vlk/hello_vk.cpp
        #vlk/hello_vk.h
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
