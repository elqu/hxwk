cmake_minimum_required(VERSION 2.8.12)

project(Hexenwerk)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_executable(hxwk main.cpp)

target_compile_options(hxwk PUBLIC "-Wall" "-Wextra" "-pedantic" "-std=c++14"
                                   "-stdlib=libc++" "-O2")

set_target_properties(hxwk PROPERTIES
                      LINK_FLAGS "-stdlib=libc++")
