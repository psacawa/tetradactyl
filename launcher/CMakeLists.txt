project(tetradactyl-launcher LANGUAGES CXX)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(
  Qt6
  COMPONENTS Widgets
  REQUIRED)
find_package(toml11 REQUIRED)
# find_package(fmt)

add_executable(tetradactyl main.cpp probe.cpp launcher.cpp config.cpp)

target_link_libraries(tetradactyl PRIVATE Qt6::Widgets)
