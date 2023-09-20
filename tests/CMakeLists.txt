# Copyright 2023 Paweł Sacawa. All rights reserved.
# test binaries should be apart from the main ones
unset(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)

add_subdirectory(launcher)

add_subdirectory(probing)

add_subdirectory(qt)

# demos for tests purposes?
add_subdirectory(demos)
