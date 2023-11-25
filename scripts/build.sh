#!/usr/bin/bash
# Copyright 2023 Paweł Sacawa. All rights reserved.

# some script which builds the target from env. var. CMAKE_DEFAULT_TARGET with cmake

target=${CMAKE_DEFAULT_TARGET:-all}
ansi_target=$target
if >/dev/null which ansi &&  [[ "${target}" != all ]]; then
  ansi_target="$(ansi --bold --yellow --no-newline ${target})"
fi
echo "Building CMake target: $ansi_target"
cmake --build build --target "${target}"
