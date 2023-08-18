enable_testing()

add_subdirectory(demos)

add_custom_target(ProcessLoadedLib.sh ALL
                  COMMAND cp "${CMAKE_SOURCE_DIR}/tests/ProcessLoadedLib.sh" .)

# dynamic probing tests

# for Qt, QT_QPA_PLATFORM=offscreen means that rendering will be done offscreen.
# Unfortunately this can't be done with package-manager distributed programs
# list /bin/*

add_test(NAME "Dynamic probe of calculator-qt5"
         COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt5 --
                 $<TARGET_FILE:tetradactyl> $<TARGET_FILE:calculator-qt5>)
set_tests_properties(
  "Dynamic probe of calculator-qt5"
  PROPERTIES LABELS "dynamic runs-gui" ENVIRONMENT QT_QPA_PLATFORM=offscreen)

add_test(NAME "Dynamic probe of calculator-qt5 with failure"
         COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt6 --
                 $<TARGET_FILE:tetradactyl> $<TARGET_FILE:calculator-qt5>)
set_tests_properties(
  "Dynamic probe of calculator-qt5 with failure"
  PROPERTIES LABELS "dynamic runs-gui" ENVIRONMENT QT_QPA_PLATFORM=offscreen
             WILL_FAIL 1)

add_test(NAME "Dynamic probe of calculator-qt6"
         COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt6 --
                 $<TARGET_FILE:tetradactyl> $<TARGET_FILE:calculator-qt6>)
set_tests_properties(
  "Dynamic probe of calculator-qt6"
  PROPERTIES LABELS "dynamic runs-gui" ENVIRONMENT QT_QPA_PLATFORM=offscreen)

add_test(NAME "Dynamic probe of calculator-qt6 with failure"
         COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt5 --
                 $<TARGET_FILE:tetradactyl> $<TARGET_FILE:calculator-qt6>)
set_tests_properties(
  "Dynamic probe of calculator-qt6 with failure"
  PROPERTIES LABELS "dynamic runs-gui" ENVIRONMENT QT_QPA_PLATFORM=offscreen
             WILL_FAIL ON)

# depends on the installed vlc loading libQt5Widgets
add_test(NAME "Dynamic probe of vlc"
         COMMAND ./ProcessLoadedLib.sh --lib libtetradactyl-qt5 --
                 $<TARGET_FILE:tetradactyl> /usr/bin/vlc)
set_tests_properties(
  "Dynamic probe of vlc" PROPERTIES LABELS "dynamic external runs-gui"
                                    ENVIRONMENT QT_QPA_PLATFORM=offscreen)
