set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(
  Qt6
  COMPONENTS Widgets Sql
  REQUIRED)
find_package(toml11 REQUIRED)
# find_package(fmt)

# file(GENERATE_FILE tetradactyl-backend-dirs )

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
  # needed to explicitly state for MOC detection - any header not in the form
  # <source>.h or <source>_p.h and therefore needed to be explicitly list if it
  # needs to be MOC'ed
  common.h)
target_link_libraries(tetradactyl PRIVATE Qt6::Widgets Qt6::Sql)

add_library(tetradactyl-dynamic-probe SHARED dynamic-probe.cpp)
set_property(TARGET tetradactyl-dynamic-probe PROPERTY BUILD_RPATH "\${ORIGIN}")
set_property(TARGET tetradactyl-dynamic-probe PROPERTY INSTALL_RPATH
                                                       "\${ORIGIN}")

install(TARGETS tetradactyl)
install(TARGETS tetradactyl-dynamic-probe)
