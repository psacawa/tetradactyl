# Copyright 2023 Paweł Sacawa. All rights reserved.
add_custom_target(ProcessLoadedLib.sh ALL
                  COMMAND cp "${CMAKE_SOURCE_DIR}/tests/ProcessLoadedLib.sh" .)

# dynamic probing tests

# for Qt, QT_QPA_PLATFORM=offscreen means that rendering will be done offscreen.
# Unfortunately this can't be done with package-manager distributed programs
# list /bin/*

if(TETRADACTYL_QT5)
  add_test(NAME "dynamic_probe_of_calculator_qt5"
           COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt5 --
                   $<TARGET_FILE:tetradactyl> $<TARGET_FILE:calculator-qt5>)
  set_tests_properties(
    "dynamic_probe_of_calculator_qt5"
    PROPERTIES LABELS "dynamic;qt5" ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()

if(TETRADACTYL_QT6)
  add_test(NAME "dynamic_probe_of_calculator_qt6"
           COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt6 --
                   $<TARGET_FILE:tetradactyl> $<TARGET_FILE:calculator-qt6>)
  set_tests_properties(
    "dynamic_probe_of_calculator_qt6"
    PROPERTIES LABELS "dynamic;qt6" ENVIRONMENT QT_QPA_PLATFORM=offscreen)

  add_test(
    NAME "dynamic_probe_of_some_dynamic_qt6_app"
    COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt6 --
            $<TARGET_FILE:tetradactyl> $<TARGET_FILE:some-dynamic-qt6-app>)
  set_tests_properties(
    "dynamic_probe_of_some_dynamic_qt6_app"
    PROPERTIES LABELS "dynamic;external;qt6" ENVIRONMENT
               QT_QPA_PLATFORM=offscreen)
endif()

# The below XFAIL  tests function primarily as a test of the test infrastructure
# itself: they prevent the test infra from reporting false positives.

if(TETRADACTYL_QT6)
  add_test(NAME "dynamic_probe_of_calculator-qt5_will_fail"
           COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt6 --
                   $<TARGET_FILE:tetradactyl> $<TARGET_FILE:calculator-qt5>)
  set_tests_properties(
    "dynamic_probe_of_calculator-qt5_will_fail"
    PROPERTIES LABELS "dynamic;xfail" ENVIRONMENT QT_QPA_PLATFORM=offscreen
               WILL_FAIL ON)
endif()

if(TETRADACTYL_QT5)
  add_test(NAME "dynamic_probe_of_calculator-qt6_will_fail"
           COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt5 --
                   $<TARGET_FILE:tetradactyl> $<TARGET_FILE:calculator-qt6>)
  set_tests_properties(
    "dynamic_probe_of_calculator-qt6_will_fail"
    PROPERTIES LABELS "dynamic;xfail" ENVIRONMENT QT_QPA_PLATFORM=offscreen
               WILL_FAIL ON)

  add_test(
    NAME "dynamic_probe_of_some_dynamic_qt6_app_will_fail"
    COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt5 --
            $<TARGET_FILE:tetradactyl> $<TARGET_FILE:some-dynamic-qt6-app>)
  set_tests_properties(
    "dynamic_probe_of_some_dynamic_qt6_app_will_fail"
    PROPERTIES LABELS "dynamic;external;xfail" ENVIRONMENT
               QT_QPA_PLATFORM=offscreen WILL_FAIL ON)
endif()
