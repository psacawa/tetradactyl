# Copyright 2023 Paweł Sacawa. All rights reserved.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include_directories("${CMAKE_SOURCE_DIR}")

# features
option(TETRADACTYL_QT5 "libtetradactyl-qt5" ON)
option(TETRADACTYL_QT6 "libtetradactyl-qt6" ON)

find_package(fmt)
find_package(toml11 REQUIRED)

set(QT_COMMON_SOURCES
    action.cpp
    common.cpp
    controller.cpp
    filter.cpp
    hint.cpp
    logging.cpp
    modelviewproxies.cpp
    overlay.cpp
    commandline.cpp
    tetradactyl.qrc)

# backward.cpp complains without this explicitly added
add_compile_options(-fPIC)

set(SUPPORTED_QT_VERSIONS "5;6")

foreach(_version ${SUPPORTED_QT_VERSIONS})
  set(_tetradactyl_backend_enabled ${TETRADACTYL_QT${_version}})
  set(_qt_version "Qt${_version}")
  set(_tetradactyl_backend "tetradactyl-qt${_version}")
  set(_tetradactyl_backend_object "tetradactyl-qt${_version}-object")

  if(${_tetradactyl_backend_enabled})
    # tetradactyl version is enabled
    message(STATUS "will build ${_tetradactyl_backend}")

    find_package(
      ${_qt_version}
      COMPONENTS Widgets
      REQUIRED)

    add_library(${_tetradactyl_backend_object} OBJECT "${QT_COMMON_SOURCES}"
                                                      probe.cpp)
    target_compile_options(${_tetradactyl_backend_object}
                           PUBLIC ${COMMON_COMPILER_OPTIONS})
    target_compile_definitions(${_tetradactyl_backend_object}
                               PUBLIC "MODULE=TETRADACTYL_QT${_version}")
    target_link_libraries(${_tetradactyl_backend_object}
                          PUBLIC ${_qt_version}::Widgets ${_qt_version}::Core)
    target_link_libraries(${_tetradactyl_backend_object}
                          PRIVATE ${_qt_version}::CorePrivate)

    add_library(${_tetradactyl_backend} SHARED
                $<TARGET_OBJECTS:${_tetradactyl_backend_object}>)
    target_link_libraries(${_tetradactyl_backend}
                          PRIVATE ${_tetradactyl_backend_object})

    # backward.cpp support
    if(DEBUG)
      include(FetchContent)
      FetchContent_Declare(
        backward
        GIT_REPOSITORY https://github.com/bombela/backward-cpp
        GIT_TAG v1.6)
      FetchContent_MakeAvailable(backward)
      add_backward(${_tetradactyl_backend})
      target_sources(${_tetradactyl_backend} PRIVATE)
      target_link_libraries(${_tetradactyl_backend} PRIVATE dwarf)
    endif()

    install(TARGETS ${_tetradactyl_backend})
  endif()
endforeach()
