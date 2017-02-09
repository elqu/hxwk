cmake_minimum_required(VERSION 2.8.12)

project(Hexenwerk)


execute_process(COMMAND llvm-config --cxxflags
                OUTPUT_VARIABLE llvm_flags_cxx)

string(STRIP ${llvm_flags_cxx} llvm_flags_cxx)
separate_arguments(llvm_flags_cxx)

execute_process(COMMAND llvm-config --ldflags
                OUTPUT_VARIABLE llvm_flags_ld)

string(STRIP ${llvm_flags_ld} llvm_flags_ld)
# This one keeps being a string

execute_process(COMMAND llvm-config --libs core
                OUTPUT_VARIABLE llvm_flags_libs)

string(STRIP ${llvm_flags_libs} llvm_flags_libs)
separate_arguments(llvm_flags_libs)

execute_process(COMMAND llvm-config --system-libs core
                OUTPUT_VARIABLE llvm_flags_libs_sys)

string(STRIP ${llvm_flags_libs_sys} llvm_flags_libs_sys)
separate_arguments(llvm_flags_libs_sys)


add_executable(hxwk main.cpp ASTInfo.cpp IRGenerator.cpp Lexer.cpp Parser.cpp)

# The C++14 option is currently being overwritten to C++11 by the LLVM flags.
# If you desire more modern features, you will have to provide some makeshift
# implementation.
set(flags_cxx_final "-Wall" "-Wextra" "-pedantic" "-std=c++14" "-O2"
                    ${llvm_flags_cxx})

target_compile_options(hxwk PUBLIC ${flags_cxx_final})

target_link_libraries(hxwk ${llvm_flags_libs_sys} ${llvm_flags_libs})

set_target_properties(hxwk PROPERTIES
                      LINK_FLAGS ${llvm_flags_ld})

set(flags_cxx_ycm "'-x',\n'c++',\n")
foreach(flag ${flags_cxx_final})
    set(flags_cxx_ycm "${flags_cxx_ycm}'${flag}',\n")
endforeach(flag)

configure_file(ycm_extra_conf.py.in "${PROJECT_SOURCE_DIR}/.ycm_extra_conf.py"
               @ONLY)
