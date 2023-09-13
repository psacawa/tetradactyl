# Copyright 2023 Paweł Sacawa. All rights reserved.

#
# Import external demos
#

function(import_qt6_examples _module)
  set("Qt6${_module}Examples_DIR" "${Qt6_DIR}/../../../examples/${_module}")
  if(NOT EXISTS ${Qt6${_module}Examples_DIR})
    message(WARNING "Qt6 Examples not available for module ${_module}")
    return()
  endif()
  execute_process(COMMAND find "${Qt6${_module}Examples_DIR}" -executable -type
                          f OUTPUT_VARIABLE Qt6_EXAMPLES_FIND_OUTPUT)
  string(REPLACE "\n" ";" Qt6_EXAMPLES "${Qt6_EXAMPLES_FIND_OUTPUT}")
  foreach(_example ${Qt6_EXAMPLES})
    # message(STATUS "${_example}")
    get_filename_component(_basename ${_example} NAME)
    set(_target "qt6-example-${_basename}")
    # global to let the tests find it
    add_executable("${_target}" IMPORTED GLOBAL)
    set_target_properties("${_target}" PROPERTIES IMPORTED_LOCATION
                                                  "${_example}")
  endforeach()
endfunction()

#
# Qt6 Tests
#

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)
import_qt6_examples(widgets)

include_directories("${CMAKE_SOURCE_DIR}")

function(add_qt6_test _name)
  set(test_name "${_name}_test")
  add_executable(${test_name} ${_name}.cpp common.cpp
                              "${CMAKE_SOURCE_DIR}/qt/${_name}.cpp")

  add_test(NAME ${test_name} COMMAND ${test_name})
  target_link_libraries(${test_name} PRIVATE Qt6::Test Qt6::Widgets)
  set_tests_properties(
    ${test_name} PROPERTIES LABELS "controller;qt6" ENVIRONMENT
                            "QT_QPA_PLATFORM=offscreen")
endfunction()

add_qt6_test(controller)
target_sources(
  controller_test
  PRIVATE "${CMAKE_SOURCE_DIR}/qt/overlay.cpp"
          "${CMAKE_SOURCE_DIR}/qt/hint.cpp"
          "${CMAKE_SOURCE_DIR}/qt/filter.cpp"
          "${CMAKE_SOURCE_DIR}/qt/logging.cpp"
          "${CMAKE_SOURCE_DIR}/qt/tetradactyl.qrc")
