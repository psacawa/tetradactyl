project(tmp LANGUAGES CXX)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(
  Qt6
  COMPONENTS Widgets
  REQUIRED)
find_package(fmt)
find_package(toml11 REQUIRED)

add_library(tetradactyl-qt SHARED controller.cpp filter.cpp hint.cpp inject.cpp
                                  tetradactyl.qrc)

target_link_libraries(tetradactyl-qt PRIVATE Qt6::Widgets)
target_link_libraries(tetradactyl-qt PRIVATE Qt6::CorePrivate) # prywatne
                                                               # nagłówki

target_link_libraries(tetradactyl-qt PRIVATE fmt)

add_subdirectory(calc-demo)
