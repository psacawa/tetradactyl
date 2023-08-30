#pragma once

#include <QFileInfo>
#include <QObject>

namespace Tetradactyl {
Q_NAMESPACE

enum WidgetBackend {
  Gtk3,
  Gtk4,
  Qt5,
  Qt6,
  None,
};
Q_ENUM_NS(WidgetBackend);

struct App {
  // Perhaps comes from XDG Desktop file;  fallback to filename
  QString name;
  QString file;
  WidgetBackend backend;

  QJsonObject toJson() const;
  static App fromJson(const QJsonObject &json);
};
} // namespace Tetradactyl
