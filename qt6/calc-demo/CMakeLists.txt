# Copyright (C) 2022 The Qt Company Ltd. SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(calculator LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

qt_standard_project_setup()

qt_add_executable(calculator button.cpp button.h calculator.cpp calculator.h
                  main.cpp)

target_link_libraries(calculator PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
# set(calculator SKIP_BUILD_RPATH YES)

install(TARGETS calculator)
