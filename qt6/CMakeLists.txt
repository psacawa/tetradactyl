project(tmp LANGUAGES CXX)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(fmt)
find_package(toml11 REQUIRED)

find_package(
  Qt6
  COMPONENTS Widgets
  REQUIRED)

find_package(
  Qt5
  COMPONENTS Widgets
  REQUIRED)

add_library(tetradactyl-qt6 SHARED controller.cpp filter.cpp hint.cpp
                                   inject.cpp tetradactyl.qrc)

add_library(tetradactyl-qt5 SHARED tetradactyl-qt5-stub.cpp)
target_link_libraries(tetradactyl-qt5 PRIVATE Qt5::Widgets)
target_link_libraries(tetradactyl-qt5 PRIVATE Qt5::CorePrivate)

target_link_libraries(tetradactyl-qt6 PRIVATE Qt6::Widgets)
# prywatne nagłówki
target_link_libraries(tetradactyl-qt6 PRIVATE Qt6::CorePrivate)

target_link_libraries(tetradactyl-qt6 PRIVATE fmt)

install(TARGETS tetradactyl-qt5)
install(TARGETS tetradactyl-qt6)
