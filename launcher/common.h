#pragma once

#include <QFileInfo>
#include <QObject>

#include <string>

using std::string;

namespace Tetradactyl {
Q_NAMESPACE

enum WidgetBackend {
  Gtk3,
  Gtk4,
  Qt5,
  Qt6,
  Unknown,
};
Q_ENUM_NS(WidgetBackend);

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
