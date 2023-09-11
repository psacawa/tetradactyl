#!/usr/bin/bash

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

$@ |
  sed "s/PASS.*/$(ansi --green --no-restore )&$(ansi --white) /g" |
  sed "s/QWARN.*/$(ansi --yellow --no-restore )&$(ansi --white) /g" |
  sed "s/QCRIT.*/$(ansi --red --no-restore )&$(ansi --white) /g" |
  sed "s/FAIL!.*/$(ansi --red --no-restore )&$(ansi --white) /g"

exit $?
