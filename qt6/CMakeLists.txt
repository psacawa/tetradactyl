project(TetradactylQt6)
cmake_minimum_required(VERSION 3.20)

# set(CMAKE_CXX_COMPILER CACHE STRING clang++)
set(CMAKE_CXX_STANDARD 17)

find_package(
  Qt6 CONFIG
  COMPONENTS Widgets
  REQUIRED)

add_library(tetradactyl-qt SHARED qabstractbutton.cpp pushbutton.cpp common.cpp)
target_link_libraries(tetradactyl-qt Qt6::Core Qt6::Gui Qt6::Widgets)
get_target_property(propvar tetradactyl-qt COMPILE_DEFINITIONS)

add_subdirectory(calc-demo)
