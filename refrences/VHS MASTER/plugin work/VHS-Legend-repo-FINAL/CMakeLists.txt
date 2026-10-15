cmake_minimum_required(VERSION 3.24)
project(VHS_Legend LANGUAGES CXX)

# Target: Adobe After Effects 23.x / Windows x64 / Visual Studio 2022
# IMPORTANT: Set AE_SDK_ROOT to the local After Effects SDK root.
set(AE_SDK_ROOT "" CACHE PATH "Path to Adobe After Effects SDK")

if(NOT WIN32)
    message(FATAL_ERROR "This plugin must be built on Windows.")
endif()

if(NOT EXISTS "${AE_SDK_ROOT}")
    message(FATAL_ERROR "AE_SDK_ROOT is not set. Point it to the After Effects SDK.")
endif()

file(GLOB_RECURSE PLUGIN_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

if(NOT PLUGIN_SOURCES)
    message(FATAL_ERROR "No plugin source files found under src/.")
endif()

add_library(VHS_Legend MODULE ${PLUGIN_SOURCES})

target_include_directories(VHS_Legend PRIVATE
    "${AE_SDK_ROOT}/Headers"
    "${AE_SDK_ROOT}/Util"
    "${AE_SDK_ROOT}/GPUUtils"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(VHS_Legend PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(VHS_Legend PRIVATE /EHsc /W3)
    target_compile_definitions(VHS_Legend PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

set_target_properties(VHS_Legend PROPERTIES
    PREFIX ""
    SUFFIX ".aex"
)
