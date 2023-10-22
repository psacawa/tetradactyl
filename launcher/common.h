// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

// This header gets included from libtetradactyl-dynamic-probe.so and in the
// launcher, the QtCore dependency is not clear. The preprocessor variable
// DYNAMIC_PROBE determines whether to avoid QtCore definitiens.
#if not defined(DYNAMIC_PROBE)
#include <QFileInfo>
#include <QObject>
#endif

#include <QFileInfo>
#include <QObject>
#include <QString>

#include <string>
#include <vector>

#include "libnames.h"

using namespace Qt::Literals::StringLiterals;
using std::string;
using std::vector;

namespace Tetradactyl {
#if not defined(DYNAMIC_PROBE)
Q_NAMESPACE
#endif

enum WidgetBackend {
  Gtk3,
  Gtk4,
  Qt5,
  Qt6,
  Unknown,
};
#if not defined(DYNAMIC_PROBE)
Q_ENUM_NS(WidgetBackend);
#endif

struct BackendData {
  WidgetBackend type;
  QString lib;
  QString tetradactylLib;
};
} // namespace Tetradactyl

extern vector<Tetradactyl::BackendData> backends;
