# Copyright 2023 Paweł Sacawa. All rights reserved.
cmake_minimum_required(VERSION 3.20)
cmake_policy(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_COMPILER clang++)

project(
  tetradactyl
  LANGUAGES CXX
  VERSION "0.1")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(TETRADACTYL_LAUNCHER "Build Tetradactyl launcher" ON)

option(TETRADACTYL_BUILD_TESTS "Build Tetradactyl tests" ON)

option(TETRADACTYL_WERROR "Adds -Werror globally" ON)

if(TETRADACTYL_BUILD_TESTS)
  enable_testing()
endif()

include(CMakePrintHelpers)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(DEBUG 1)
  add_compile_definitions("DEBUG")
endif()

# put all binaries/libs in the same place
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(COMMON_COMPILER_OPTIONS
    -Wall
    -Wuninitialized
    -Wmost
    -Wextra
    -Wno-unused-parameter
    -Werror=return-type
    -Werror=missing-include-dirs)
if(TETRADACTYL_WERROR)
  list(APPEND COMMON_COMPILER_OPTIONS -Werror)
endif()

add_compile_definitions("TETRADACTYL_VERSION=${CMAKE_PROJECT_VERSION}")

# TODO 16/08/20 psacawa: finish this add_subdirectory(common)
# include_directories(common)

if(TETRADACTYL_LAUNCHER)
  # libtetradactyl + libtetradactyl-dynamic
  add_subdirectory(launcher)
endif()

add_subdirectory(qt)

# preferred build system for gtk is meson add_subdirectory(gtk4)

if(TETRADACTYL_BUILD_TESTS)
  add_subdirectory(tests)
endif()
