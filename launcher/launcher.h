#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListView>
#include <QMainWindow>
#include <QPushButton>
#include <QString>
#include <Qt>

#include <cstdlib>
#include <qfileinfo.h>
#include <qtmetamacros.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

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
  QFileInfo file;
  WidgetBackend backend;

  QJsonObject toJson() const;
  static App fromJson(const QJsonObject &json);
};

class ApplicationListModel;

class Launcher : public QMainWindow {
  Q_OBJECT
public:
  Launcher();
  virtual ~Launcher() {}

private:
  void createLayout();
  QWidget *central;
  QLayout *layout;
  QWidget *bottom;

  QLayout *bottomLayout;
  QPushButton *launchButton;
  QPushButton *addButton;
  QPushButton *refreshButton;
  QPushButton *exitButton;

  ApplicationListModel *model;
  QListView *view;
};

class ApplicationListModel : public QAbstractListModel {
  Q_OBJECT
public:
  ApplicationListModel();
  virtual ~ApplicationListModel() {}
  virtual int rowCount(const QModelIndex &parent) const override;
  virtual QVariant data(const QModelIndex &index,
                        int role = Qt::DisplayRole) const override;
public slots:
  void addTetradactylApp(QFileInfo file, WidgetBackend backend);
  void launch(const QModelIndex &index);
  void cacheTetradactylApps();

private:
  vector<App *> apps;
  App *selectedApp;
};
} // namespace Tetradactyl
