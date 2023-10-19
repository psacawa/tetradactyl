# Copyright 2023 Paweł Sacawa. All rights reserved.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/launcher")

# pozwala na e.g. #include <launcher/common.h>
include_directories("${CMAKE_SOURCE_DIR}")

function(add_launcher_basic_test _name)
  set(test_name "${_name}_test")
  add_executable(
    ${test_name} ${_name}.cpp "${CMAKE_SOURCE_DIR}/launcher/${_name}.cpp"
                 "${CMAKE_SOURCE_DIR}/launcher/common.cpp")

  add_test(NAME ${test_name} COMMAND ${test_name})
  target_link_libraries(${test_name} PRIVATE Qt6::Test)
  set_tests_properties(${test_name} PROPERTIES LABELS launcher)
endfunction()

function(add_launcher_unit_test _test_name)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "" "LABELS;SOURCES" "")
  list(TRANSFORM ARG_SOURCES PREPEND "${CMAKE_SOURCE_DIR}/launcher/")
  add_executable(
    ${_test_name} "${_test_name}.cpp"
                  $<TARGET_OBJECTS:tetradactyl-launcher-test-object>)
  add_test(NAME ${_test_name} COMMAND ${_test_name})
  target_link_libraries(${_test_name} PRIVATE tetradactyl-launcher-test-object)
  set_tests_properties(${_test_name} PROPERTIES LABELS ${ARG_LABELS})
endfunction()

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Test)

set(LAUNCHER_SOURCES
    "${CMAKE_SOURCE_DIR}/launcher/probe.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/launcher.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/config.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/utils.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/applicationtablemodel.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/waitingspinnerwidget.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/launcherwindow.ui"
    "${CMAKE_SOURCE_DIR}/launcher/common.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/abi.cpp")

add_library(tetradactyl-launcher-test-object OBJECT ${LAUNCHER_SOURCES})
target_link_libraries(tetradactyl-launcher-test-object PUBLIC Qt::Widgets
                                                              Qt::Sql Qt::Test)

add_launcher_unit_test(probe_test SOURCES probe.cpp common.cpp LABELS launcher)

# TODO 18/10/20 psacawa: replace this legacy cmake
add_launcher_basic_test(applicationtablemodel)
target_sources(
  applicationtablemodel_test
  PRIVATE common.cpp "${CMAKE_SOURCE_DIR}/launcher/common.cpp"
          "${CMAKE_SOURCE_DIR}/launcher/abi.cpp"
          "${CMAKE_SOURCE_DIR}/launcher/utils.cpp")
target_link_libraries(applicationtablemodel_test PRIVATE Qt6::Sql Qt6::Widgets)
