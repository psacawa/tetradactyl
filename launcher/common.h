// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

// This header gets included from libtetradactyl-dynamic-probe.so and in the
// launcher, the QtCore dependency is not clear. The preprocessor variable
// DYNAMIC_PROBE determines whether to avoid QtCore definitiens.
#ifndef DYNAMIC_PROBE
#include <QFileInfo>
#include <QObject>
#endif

#include <QFileInfo>
#include <QObject>

#include <string>
#include <vector>

#include "libnames.h"

using std::string;
using std::vector;

namespace Tetradactyl {
#ifndef DYNAMIC_PROBE
Q_NAMESPACE
#endif

enum WidgetBackend {
  Gtk3,
  Gtk4,
  Qt5,
  Qt6,
  Unknown,
};
#ifndef DYNAMIC_PROBE
Q_ENUM_NS(WidgetBackend);
#endif

struct BackendData {
  WidgetBackend type;
  string lib;
  string tetradactylLib;
};

struct App {
  // Perhaps comes from XDG Desktop file;  fallback to filename
  QString name;
  QString file;
  WidgetBackend backend;

  QJsonObject toJson() const;
  static App fromJson(const QJsonObject &json);
};
} // namespace Tetradactyl

extern vector<Tetradactyl::BackendData> backends;
