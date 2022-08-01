cmake_minimum_required(VERSION 3.22.1)
project("mergen")
add_library(mergen SHARED main.cpp)
target_link_libraries(mergen)
