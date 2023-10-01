# Copyright 2023 Paweł Sacawa. All rights reserved. test binaries should be
# apart from the main ones
unset(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)

option(TETRADCTYL_CODE_COVERAGE "Build for code coverage" OFF)
if(TETRADCTYL_CODE_COVERAGE)
  include(CodeCoverage)
  append_coverage_compiler_flags()
endif()

add_subdirectory(launcher)

add_subdirectory(probing)

add_subdirectory(qt)

# demos for tests purposes?
add_subdirectory(demos)
