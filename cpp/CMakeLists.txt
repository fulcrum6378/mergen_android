cmake_minimum_required(VERSION 3.4.1)
project(Mergen VERSION ${VERSION})

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall")
add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR=1)

add_library(main SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/aud/commons.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/aud/microphone.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/aud/speaker.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/hpt/touchscreen.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/mem/manifest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mem/queuer.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/mov/vibrator.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/rew/rewarder.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/vis/camera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vis/colouring.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vis/segmentation.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/vlk/hello_vk.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vlk/compute_vk.cpp
        )

add_subdirectory(third_party/boost/thread)
add_subdirectory(third_party/boost/lexical_cast)
add_subdirectory(third_party/boost/type_traits)
add_subdirectory(third_party/boost/throw_exception)
add_subdirectory(third_party/boost/static_assert)
add_subdirectory(third_party/boost/range)
add_subdirectory(third_party/boost/utility)
add_subdirectory(third_party/boost/tuple)
add_subdirectory(third_party/boost/regex)
add_subdirectory(third_party/boost/assert)
add_subdirectory(third_party/boost/predef)
add_subdirectory(third_party/boost/config)
add_subdirectory(third_party/boost/preprocessor)
add_subdirectory(third_party/boost/optional)
add_subdirectory(third_party/boost/mpl)
add_subdirectory(third_party/boost/core)
add_subdirectory(third_party/boost/iterator)
add_subdirectory(third_party/boost/detail)
add_subdirectory(third_party/boost/conversion)
add_subdirectory(third_party/boost/container_hash)
add_subdirectory(third_party/boost/concept_check)
add_subdirectory(third_party/boost/array)
add_subdirectory(third_party/boost/numeric/conversion)
add_subdirectory(third_party/boost/integer)
add_subdirectory(third_party/boost/container)
add_subdirectory(third_party/boost/move)
add_subdirectory(third_party/boost/intrusive)
add_subdirectory(third_party/boost/mp11)
add_subdirectory(third_party/boost/describe)
add_subdirectory(third_party/boost/algorithm)
add_subdirectory(third_party/boost/unordered)
add_subdirectory(third_party/boost/function)
add_subdirectory(third_party/boost/exception)
add_subdirectory(third_party/boost/smart_ptr)
add_subdirectory(third_party/boost/bind)
add_subdirectory(third_party/boost/winapi)
add_subdirectory(third_party/boost/system)
add_subdirectory(third_party/boost/variant2)
add_subdirectory(third_party/boost/io)
add_subdirectory(third_party/boost/fusion)
add_subdirectory(third_party/boost/functional)
add_subdirectory(third_party/boost/typeof)
add_subdirectory(third_party/boost/function_types)
add_subdirectory(third_party/boost/date_time)
add_subdirectory(third_party/boost/tokenizer)
add_subdirectory(third_party/boost/atomic)
add_subdirectory(third_party/boost/align)
add_subdirectory(third_party/boost/chrono)
add_subdirectory(third_party/boost/ratio)
add_subdirectory(third_party/boost/rational)

target_link_libraries(main
        android      # for ANativeWindow
        boost_thread
        camera2ndk
        log          # for <android/log.h>
        mediandk     # for AImageReader
        OpenSLES
        vulkan
        )
