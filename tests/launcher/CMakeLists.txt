set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/launcher")

# pozwala na e.g. #include <launcher/common.h>
include_directories("${CMAKE_SOURCE_DIR}")

function(add_launcher_test _name)
  set(test_name "${_name}_test")
  add_executable(
    ${test_name} ${_name}.cpp "${CMAKE_SOURCE_DIR}/launcher/${_name}.cpp"
                 "${CMAKE_SOURCE_DIR}/launcher/common.h")

  add_test(NAME ${test_name} COMMAND ${test_name})
  target_link_libraries(${test_name} PRIVATE Qt6::Test)
endfunction()

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Test)

add_launcher_test(applicationtablemodel)
target_sources(applicationtablemodel_test PRIVATE common.cpp)
target_link_libraries(applicationtablemodel_test PRIVATE Qt6::Sql Qt6::Widgets)

add_executable(functionallauncher_test functionallauncher.cpp common.cpp)
add_test(NAME functionallauncher_test COMMAND functionallauncher_test)
target_link_libraries(functionallauncher_test PRIVATE Qt6::Test Qt6::Sql
                                                      Qt6::Widgets)
