# Copyright 2023 Paweł Sacawa. All rights reserved.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/launcher")

# pozwala na e.g. #include <launcher/common.h>
include_directories("${CMAKE_SOURCE_DIR}")

function(add_launcher_unit_test _test_name)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "" "LABELS;SOURCES" "")
  list(TRANSFORM ARG_SOURCES PREPEND "${CMAKE_SOURCE_DIR}/launcher/")
  add_executable(
    ${_test_name} "${_test_name}.cpp"
                  $<TARGET_OBJECTS:tetradactyl-launcher-test-object>)
  target_include_directories(
    ${_test_name}
    PRIVATE
      $<TARGET_PROPERTY:tetradactyl-launcher-test-object,INCLUDE_DIRECTORIES>)
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
    "${CMAKE_SOURCE_DIR}/launcher/applicationmodel.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/app.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/waitingspinnerwidget.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/launcherwindow.ui"
    "${CMAKE_SOURCE_DIR}/launcher/common.cpp"
    "${CMAKE_SOURCE_DIR}/launcher/abi.cpp")

add_library(tetradactyl-launcher-test-object OBJECT ${LAUNCHER_SOURCES})
target_link_libraries(tetradactyl-launcher-test-object PUBLIC Qt::Widgets
                                                              Qt::Sql Qt::Test)

pkg_config_add_to_target(tetradactyl-launcher-test-object GIOMM)

add_launcher_unit_test(probe_test LABELS launcher)

add_launcher_unit_test(applicationmodel_test LABELS launcher)

add_launcher_unit_test(launcher_e2e_test LABELS launcher e2e)

add_launcher_unit_test(app_test LABELS launcher)

add_launcher_unit_test(utils_test LABELS launcher)
