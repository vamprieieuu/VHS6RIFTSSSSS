cmake_minimum_required(VERSION 3.24)
project(VHS6RIFT LANGUAGES CXX RC)

if(NOT WIN32)
  message(FATAL_ERROR "VHS6RIFT currently targets Windows x64 / After Effects 23.x.")
endif()
if(NOT MSVC)
  message(FATAL_ERROR "Use Visual Studio 2022 / MSVC for the Windows PiPL build.")
endif()

set(AE_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/extern/AfterEffectsSDK" CACHE PATH "Root of the Adobe After Effects SDK or its Examples folder")
if(EXISTS "${AE_SDK_ROOT}/Headers/AE_Effect.h")
  set(AE_INCLUDE_ROOT "${AE_SDK_ROOT}")
elseif(EXISTS "${AE_SDK_ROOT}/Examples/Headers/AE_Effect.h")
  set(AE_INCLUDE_ROOT "${AE_SDK_ROOT}/Examples")
else()
  message(FATAL_ERROR "Adobe AE SDK not found. Expected Headers/AE_Effect.h or Examples/Headers/AE_Effect.h under ${AE_SDK_ROOT}.")
endif()
set(AE_HEADERS "${AE_INCLUDE_ROOT}/Headers")
set(PIPL_TOOL "${AE_SDK_ROOT}/Resources/PiPLtool.exe")
if(NOT EXISTS "${PIPL_TOOL}")
  set(PIPL_TOOL "${AE_INCLUDE_ROOT}/Resources/PiPLtool.exe")
endif()
if(NOT EXISTS "${PIPL_TOOL}")
  message(FATAL_ERROR "PiPLtool.exe not found. Expected Resources/PiPLtool.exe beside the SDK root or its Examples folder.")
endif()

set(PIPL_R "${CMAKE_CURRENT_SOURCE_DIR}/resources/VHS6RIFTPiPL.r")
set(PIPL_RR "${CMAKE_CURRENT_BINARY_DIR}/VHS6RIFTPiPL.rr")
set(PIPL_RRC "${CMAKE_CURRENT_BINARY_DIR}/VHS6RIFTPiPL.rrc")
set(PIPL_RC "${CMAKE_CURRENT_BINARY_DIR}/VHS6RIFTPiPL.rc")
add_custom_command(OUTPUT "${PIPL_RC}"
  COMMAND "${CMAKE_COMMAND}"
    "-DPIPL_TOOL=${PIPL_TOOL}"
    "-DCXX_COMPILER=${CMAKE_CXX_COMPILER}"
    "-DAE_HEADERS=${AE_HEADERS}"
    "-DPIPL_R=${PIPL_R}"
    "-DPIPL_RR=${PIPL_RR}"
    "-DPIPL_RRC=${PIPL_RRC}"
    "-DPIPL_RC=${PIPL_RC}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/tools/compile_pipl.cmake"
  DEPENDS "${PIPL_R}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/compile_pipl.cmake"
  COMMENT "Generating VHS6RIFT PiPL resource"
  VERBATIM)
add_custom_target(VHS6RIFT_PiPL DEPENDS "${PIPL_RC}")

add_library(VHS6RIFT MODULE src/VHS6RIFT.cpp "${PIPL_RC}")
add_dependencies(VHS6RIFT VHS6RIFT_PiPL)
target_include_directories(VHS6RIFT PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${AE_HEADERS}" "${AE_HEADERS}/SP" "${AE_INCLUDE_ROOT}/Util" "${AE_INCLUDE_ROOT}/Examples/Headers")
target_compile_features(VHS6RIFT PRIVATE cxx_std_17)
target_compile_definitions(VHS6RIFT PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(VHS6RIFT PRIVATE /EHsc /W3 /permissive-)
set_target_properties(VHS6RIFT PROPERTIES PREFIX "" SUFFIX ".aex" OUTPUT_NAME "VHS6RIFT")
set_target_properties(VHS6RIFT PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS OFF)
