# Copyright (C) 2022 The Qt Company Ltd. SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(calculator LANGUAGES CXX)

# qt_standard_project_setup()

add_executable(calculator-qt5 button.cpp button.h calculator.cpp calculator.h
                  main.cpp)
qt_add_executable(calculator-qt6 button.cpp button.h calculator.cpp calculator.h
                  main.cpp)


target_link_libraries(calculator-qt5 PRIVATE Qt5::Widgets)
target_link_libraries(calculator-qt6 PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

install(TARGETS calculator-qt5)
install(TARGETS calculator-qt6)
