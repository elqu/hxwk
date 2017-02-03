cmake_minimum_required(VERSION 2.8.12)

project(Hexenwerk)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_COMPILER "clang++")

configure_file(ycm_extra_conf.py.in "${PROJECT_SOURCE_DIR}/.ycm_extra_conf.py" @ONLY)

add_executable(hxwk main.cpp)

target_compile_options(hxwk PUBLIC "-Wall" "-Wextra" "-pedantic" "-std=c++14"
                                   "-stdlib=libc++" "-O2")

set_target_properties(hxwk PROPERTIES
                      LINK_FLAGS "-stdlib=libc++")
