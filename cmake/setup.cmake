# Copyright 2023 Paweł Sacawa. All rights reserved.

find_package(PkgConfig REQUIRED)
macro(pkg_config_add_to_target _target _prefix)
  target_link_libraries(${_target} PRIVATE ${${_prefix}_LINK_LIBRARIES})
  target_link_directories(${_target} PRIVATE ${${_prefix}_LIBRARY_DIRS})
  target_include_directories(${_target} PRIVATE ${${_prefix}_INCLUDE_DIRS})
endmacro()
pkg_check_modules(GIOMM giomm-2.68)

# backward.cpp support
include(FetchContent)
FetchContent_Declare(
  backward
  GIT_REPOSITORY https://github.com/bombela/backward-cpp
  GIT_TAG v1.6)
FetchContent_MakeAvailable(backward)
