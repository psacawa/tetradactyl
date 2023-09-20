#!/usr/bin/bash
# Copyright 2023 Paweł Sacawa. All rights reserved.

# Dumbass script to patch QtTest's lack of colour terminal output
# To use:
#
# add_custom_target(
#   colourize_output.sh ALL
#   COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/colourize_output.sh" .
#   DEPENDS colourize_output.sh)
# ...
# add_executable(sometest somewindow.cpp "${CMAKE_SOURCE_DIR}/somewindow.cpp")
# add_test(NAME sometest COMMAND colourize_output.sh $<TARGET_FILE:sometest>)


# return value must come from original command
set -o pipefail

function sigabrt_handler () {
  echo got SIGABRT 2>/dev/null
  exit 1
}

trap sigabrt_handler ABRT 

$@ |
  sed "s/PASS.*/$(ansi --green --no-restore )&$(ansi --reset-color) /g" |
  sed "s/QWARN.*/$(ansi --yellow --no-restore )&$(ansi --reset-color) /g" |
  sed "s/QCRIT.*/$(ansi --red --no-restore )&$(ansi --reset-color) /g" |
  sed "s/QFATAL.*/$(ansi --red --no-restore )&$(ansi --reset-color) /g" |
  sed "s/FAIL!.*/$(ansi --red-intense  --bold --no-restore )&$(ansi --reset-color) /g" |
  sed "s/.*Finished testing.*/$(ansi --blue  --no-restore )&$(ansi --reset-color) /g"

exit $?
