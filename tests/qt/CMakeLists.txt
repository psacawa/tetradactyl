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

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(TETRADACTYL_QT6)
  # Qt6 Tests

  find_package(Qt6 REQUIRED COMPONENTS Widgets Test)
  import_qt6_examples(widgets)

  include_directories("${CMAKE_SOURCE_DIR}")

  function(add_qt6_test _test_name)
    cmake_parse_arguments(PARSE_ARGV 0 ADD_QT6_TEST "" LABELS "")
    add_executable(${_test_name} ${_test_name}.cpp common.cpp)

    set(_test_env "QT_QPA_PLATFORM=offscreen")

    # LD_PRELOAD=libtetradactyl-qt6.so if inject is a label
    list(FIND ADD_QT6_TEST_LABELS inject _test_needs_preload)
    if(${_test_needs_preload} GREATER_EQUAL 0)
      list(APPEND _test_env "LD_PRELOAD=$<TARGET_FILE:tetradactyl-qt6>")
    endif()

    add_test(NAME ${_test_name} COMMAND ${_test_name})
    target_link_libraries(${_test_name} PRIVATE Qt6::Test Qt6::Widgets)
    set_tests_properties(${_test_name} PROPERTIES ENVIRONMENT "${_test_env}")
    message(STATUS "${ADD_QT6_TEST_LABELS}")
    set_tests_properties(${_test_name} PROPERTIES LABELS
                                                  "${ADD_QT6_TEST_LABELS}")
  endfunction()

  add_qt6_test(basiccontroller_test LABELS "controller;qt6")
  target_sources(
    basiccontroller_test
    PRIVATE "${CMAKE_SOURCE_DIR}/qt/action.cpp"
            "${CMAKE_SOURCE_DIR}/qt/controller.cpp"
            "${CMAKE_SOURCE_DIR}/qt/filter.cpp"
            "${CMAKE_SOURCE_DIR}/qt/hint.cpp"
            "${CMAKE_SOURCE_DIR}/qt/modelviewproxies.cpp"
            "${CMAKE_SOURCE_DIR}/qt/logging.cpp"
            "${CMAKE_SOURCE_DIR}/qt/overlay.cpp"
            "${CMAKE_SOURCE_DIR}/qt/tetradactyl.qrc")

  # TODO 15/09/20 psacawa: how to gracefully test external examples?
  # add_qt6_test(tabbar_test LABELS "tabbar;linux;inject;controller;qt6")
  # target_sources( tabbar_test PRIVATE "${CMAKE_SOURCE_DIR}/qt/action.cpp"
  # "${CMAKE_SOURCE_DIR}/qt/controller.cpp" "${CMAKE_SOURCE_DIR}/qt/filter.cpp"
  # "${CMAKE_SOURCE_DIR}/qt/hint.cpp" "${CMAKE_SOURCE_DIR}/qt/logging.cpp"
  # "${CMAKE_SOURCE_DIR}/qt/overlay.cpp"
  # "${CMAKE_SOURCE_DIR}/qt/tetradactyl.qrc")
endif()
