cmake_minimum_required(VERSION 3.22.1)
project(Mergen VERSION ${VERSION})

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wpedantic")
add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR=1)

add_library(main SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        global.hpp
        global_vk.hpp

        aud/beeping.hpp
        aud/microphone.cpp
        aud/microphone.hpp
        aud/speaker.cpp
        aud/speaker.hpp

        hpt/painful_point.hpp
        hpt/touchscreen.cpp
        hpt/touchscreen.hpp

        mov/shaking.hpp
        mov/vibrator.hpp

        rew/criterion.hpp
        rew/expression.hpp
        rew/rewarder.cpp
        rew/rewarder.hpp

        scm/manifest.hpp

        #vis/bitmap.hpp
        vis/camera.cpp
        vis/camera.hpp
        vis/colouring.hpp
        vis/edge_detection.hpp
        vis/edge_detection.cpp
        #[[vis/memory.cpp
        vis/memory.hpp]]
        vis/perception.hpp
        #[[vis/segment.hpp
        vis/segmentation_a.cpp
        vis/segmentation_a.hpp]]
        vis/segmentation_b.cpp
        vis/segmentation_b.hpp
        #[[vis/visual_stm.cpp
        vis/visual_stm.hpp]]
)

# Paths
target_include_directories(main PUBLIC
        $ENV{ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include  # NDK for CLion
        #include  # third-party dependencies
)
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")

# NDK headers
target_link_libraries(main
        aaudio       # AAudioXXXXX
        android      # <android/native_window(_jni).h>::ANativeWindow(_fromSurface)
        camera2ndk   # ACameraXXXXX
        log          # <android/log.h>::__android_log_assert
        mediandk     # AImageReader
        vulkan       # Vulkan headers
)

# Dependencies
#add_subdirectory()
