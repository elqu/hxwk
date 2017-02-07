cmake_minimum_required(VERSION 2.8.12)

project(Hexenwerk)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
configure_file(ycm_extra_conf.py.in "${PROJECT_SOURCE_DIR}/.ycm_extra_conf.py"
               @ONLY)

set(CMAKE_CXX_COMPILER "clang++")


execute_process(COMMAND llvm-config --cxxflags
                OUTPUT_VARIABLE llvm_flags_cxx)

string(STRIP ${llvm_flags_cxx} llvm_flags_cxx)
separate_arguments(llvm_flags_cxx)

execute_process(COMMAND llvm-config --ldflags
                OUTPUT_VARIABLE llvm_flags_ld)

string(STRIP ${llvm_flags_ld} llvm_flags_ld)
# This one stays a string

execute_process(COMMAND llvm-config --libs core
                OUTPUT_VARIABLE llvm_flags_libs)

string(STRIP ${llvm_flags_libs} llvm_flags_libs)
separate_arguments(llvm_flags_libs)

execute_process(COMMAND llvm-config --system-libs core
                OUTPUT_VARIABLE llvm_flags_libs_sys)

string(STRIP ${llvm_flags_libs_sys} llvm_flags_libs_sys)
separate_arguments(llvm_flags_libs_sys)


add_executable(hxwk main.cpp)

# The C++14 option is overwritten to C++11 by the LLVM flags. If you desire
# more modern features, you have to provide some makeshift implementation.
target_compile_options(hxwk PUBLIC "-Wall" "-Wextra" "-pedantic" "-std=c++14"
                                   "-stdlib=libc++" "-O2" ${llvm_flags_cxx})

target_link_libraries(hxwk ${llvm_flags_libs_sys} ${llvm_flags_libs})

set_target_properties(hxwk PROPERTIES
                      LINK_FLAGS "-stdlib=libc++ ${llvm_flags_ld}")
