cmake_minimum_required(VERSION 3.20)
cmake_policy(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(
  tetradactyl
  LANGUAGES CXX
  VERSION "0.1")

enable_testing()

# cmake kruft to put all *.so in one place
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_compile_options(-Wall -Werror=return-type -Werror=unknown-pragmas)
add_compile_definitions("TETRADACTYL_VERSION=${CMAKE_PROJECT_VERSION}")

# TODO 16/08/20 psacawa: finish this add_subdirectory(common)
# include_directories(common)

# libtetradactyl + libtetradactyl-dynamic
add_subdirectory(launcher)

add_subdirectory(qt6)

add_subdirectory(tests)

# preferred build system for gtk is meson add_subdirectory(gtk4)
