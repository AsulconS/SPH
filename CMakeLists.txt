cmake_minimum_required(VERSION 3.12)

# Disable CYGWIN Legacy Warning
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

# Set all version configuration
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)
set(SO_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
set(FULL_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# Project info
project(SPH VERSION ${FULL_VERSION})

# Set C++11 as the standard
set(CMAKE_CXX_STANDARD 11)

# Output Dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}>)

# Building Macro
macro(build_cpp_source filename)
    add_executable(${filename} ${filename}.cpp)
    target_compile_definitions(${filename}
    PRIVATE
        __STDC_LIB_EXT1__
    )
    target_include_directories(${filename} PRIVATE include)
    target_include_directories(${filename} PRIVATE include/HSGIL/external)
    if(WIN32)
        target_link_libraries(${filename} LINK_PUBLIC hsgil opengl32)
    else()
        target_link_libraries(${filename} LINK_PUBLIC hsgil)
    endif()
endmacro(build_cpp_source)

build_cpp_source(fluid)
build_cpp_source(main)
build_cpp_source(old)
