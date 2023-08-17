set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(
  Qt6
  COMPONENTS Widgets
  REQUIRED)
find_package(toml11 REQUIRED)
# find_package(fmt)

# file(GENERATE_FILE tetradactyl-backend-dirs )

add_executable(tetradactyl main.cpp probe.cpp launcher.cpp config.cpp utils.cpp)
target_link_libraries(tetradactyl PRIVATE Qt6::Widgets)

add_library(tetradactyl-dynamic-probe SHARED dynamic-probe.cpp)
set_property(TARGET tetradactyl-dynamic-probe PROPERTY BUILD_RPATH "\${ORIGIN}/../lib")
set_property(TARGET tetradactyl-dynamic-probe PROPERTY INSTALL_RPATH "\${ORIGIN}/../lib")

install(TARGETS tetradactyl)
install(TARGETS tetradactyl-dynamic-probe)
