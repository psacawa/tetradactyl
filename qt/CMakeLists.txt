# Copyright 2023 Paweł Sacawa. All rights reserved.
project(tmp LANGUAGES CXX)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(fmt)
find_package(toml11 REQUIRED)

if(TETRADACTYL_QT5)
  # tetradactyl-qt5

  find_package(
    Qt5
    COMPONENTS Widgets
    REQUIRED)

  add_library(
    tetradactyl-qt5 SHARED
    action.cpp
    controller.cpp
    filter.cpp
    hint.cpp
    overlay.cpp
    logging.cpp
    probe.cpp
    tetradactyl.qrc)

  target_link_libraries(tetradactyl-qt5 PRIVATE Qt5::Widgets)
  target_link_libraries(tetradactyl-qt5 PRIVATE Qt5::CorePrivate)

  install(TARGETS tetradactyl-qt5)
endif()

if(TETRADACTYL_QT6)
  # tetradactyl-qt6

  find_package(
    Qt6
    COMPONENTS Widgets
    REQUIRED)

  add_library(
    tetradactyl-qt6 SHARED
    action.cpp
    controller.cpp
    filter.cpp
    hint.cpp
    overlay.cpp
    logging.cpp
    probe.cpp
    tetradactyl.qrc)

  target_link_libraries(tetradactyl-qt6 PRIVATE Qt6::Widgets)
  target_link_libraries(tetradactyl-qt6 PRIVATE Qt6::CorePrivate)

  install(TARGETS tetradactyl-qt6)
endif()
