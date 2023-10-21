// Copyright 2023 Paweł Sacawa. All rights reserved.

#include <QIcon>

#include <csignal>

#include <backward.hpp>

#include "common.h"
#include "libnames.h"

using std::vector;
using Tetradactyl::BackendData;
using Tetradactyl::WidgetBackend;

#if DEBUG
// n.b. failed asserts are SIGABRT
backward::SignalHandling sh;
#endif

class QtInitTheme {
public:
  QtInitTheme() { QIcon::setThemeName("breeze"); }
};
QtInitTheme init;

// Map of widget lib names to tetradactyl backend libnames.Note that this gets
// used in a __attribute__((constructor)) context, so any C++ classes,
// specifically STL maps are forbidden here.
// TODO 16/08/20 psacawa: support more complex detection mechanism.

vector<BackendData> backends = {
    {WidgetBackend::Gtk3, GTK3_LIB, GTK3_TETRADACTYL_LIB},
    {WidgetBackend::Gtk4, GTK4_LIB, GTK4_TETRADACTYL_LIB},
    {WidgetBackend::Qt5, QT5_LIB, QT5_TETRADACTYL_LIB},
    {WidgetBackend::Qt6, QT6_LIB, QT6_TETRADACTYL_LIB}};
