find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(some-dynamic-qt6-app-ui SHARED ui.cpp)
target_link_libraries(some-dynamic-qt6-app-ui PRIVATE Qt6::Widgets)
add_executable(some-dynamic-qt6-app main.cpp)
set_property(TARGET some-dynamic-qt6-app PROPERTY BUILD_RPATH "\${ORIGIN}")
