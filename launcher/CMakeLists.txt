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

# needed to help RCC generate header which can find launcher/waitingspinnerwidget.h
include_directories("${CMAKE_SOURCE_DIR}")

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
  applicationmodel.cpp
  waitingspinnerwidget.cpp
  launcherwindow.ui
  common.cpp
  abi.cpp
  app.cpp
  launcher.qrc)

target_link_libraries(tetradactyl PRIVATE Qt6::Widgets Qt6::Sql)
pkg_config_add_to_target(tetradactyl GIOMM)

if(DEBUG)
  add_backward(tetradactyl)
  target_sources(tetradactyl PRIVATE)
  target_link_libraries(tetradactyl PRIVATE dwarf)
endif()

add_library(tetradactyl-dynamic-probe SHARED dynamic-probe.cpp)
target_link_libraries(tetradactyl-dynamic-probe PRIVATE fmt)
set_property(TARGET tetradactyl-dynamic-probe PROPERTY BUILD_RPATH "\${ORIGIN}")
set_property(TARGET tetradactyl-dynamic-probe PROPERTY INSTALL_RPATH
                                                       "\${ORIGIN}")

install(TARGETS tetradactyl)
install(TARGETS tetradactyl-dynamic-probe)
