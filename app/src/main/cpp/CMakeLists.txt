cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)
set(COMMON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../common)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
add_library(main SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/android_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camera_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camera_engine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camera_listeners.cpp
        ${COMMON_SOURCE_DIR}/utils/camera_utils.cpp)

# add include path
target_include_directories(main PRIVATE ${COMMON_SOURCE_DIR})

# add lib dependencies
target_link_libraries(main dl android log m camera2ndk)
