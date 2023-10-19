# Copyright 2023 Paweł Sacawa. All rights reserved.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(
  Qt6
  COMPONENTS Widgets Sql
  REQUIRED)
find_package(toml11 REQUIRED)
find_package(fmt REQUIRED)

find_package(PkgConfig REQUIRED)
macro(pkg_config_add_to_target _target _prefix)
  target_link_libraries(${_target} PRIVATE ${${_prefix}_LINK_LIBRARIES})
  target_link_directories(${_target} PRIVATE ${${_prefix}_LIBRARY_DIRS})
  target_include_directories(${_target} PRIVATE ${${_prefix}_INCLUDE_DIRS})
endmacro()
pkg_check_modules(GIOMM giomm-2.68)

# file(GENERATE_FILE tetradactyl-backend-dirs )

# Any header with Q_ENUM/Q_NAMESPACE etc. needs to by explicitly stated for MOC
# detection - any header not in the form <source>.h or <source>_p.h and
# therefore needed to be explicitly list if it needs to be MOC'ed
add_executable(
  tetradactyl
  main.cpp
  probe.cpp
  launcher.cpp
  config.cpp
  utils.cpp
  applicationtablemodel.cpp
  waitingspinnerwidget.cpp
  launcherwindow.ui
  common.cpp
  abi.cpp
  app.cpp)

target_link_libraries(tetradactyl PRIVATE Qt6::Widgets Qt6::Sql)
pkg_config_add_to_target(tetradactyl GIOMM)

add_library(tetradactyl-dynamic-probe SHARED dynamic-probe.cpp)
target_link_libraries(tetradactyl-dynamic-probe PRIVATE fmt)
set_property(TARGET tetradactyl-dynamic-probe PROPERTY BUILD_RPATH "\${ORIGIN}")
set_property(TARGET tetradactyl-dynamic-probe PROPERTY INSTALL_RPATH
                                                       "\${ORIGIN}")

install(TARGETS tetradactyl)
install(TARGETS tetradactyl-dynamic-probe)
