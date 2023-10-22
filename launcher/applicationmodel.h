// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <QScopedPointer>
#include <Qt>

#include "app.h"
#include "common.h"
#include "probe.h"

using std::unique_ptr;

#define TETRADACTYL_APPS_DB_FILE "apps.json"

enum {
  BackendRole = Qt::UserRole,
};

QFile appDatabaseFile();

namespace Tetradactyl {

class AppAlreadyPresent : public std::invalid_argument {
  using std::invalid_argument::invalid_argument;
};

class ApplicationModel : public QAbstractListModel {
  Q_OBJECT
public:
  ApplicationModel(QObject *parent = nullptr);
  virtual ~ApplicationModel() {}

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  bool contains(AbstractApp *app);

signals:
  void appDatabaseBuilt(int numNewAppsAdded);

public slots:
  WidgetBackend probeAndAddApp(QString file);
  void initiateBuildAppDatabase();
  void cancelBuildAppDatabase();
  void saveDatabase();
  void tryLoadDatabase();
  void resetDatabase();
  void tryAddApp(AbstractApp *app);

  void launch(const QModelIndex &index);

private:
  QList<AbstractApp *> p_apps;
  unique_ptr<ProbeThread> probeThread = nullptr;
};

} // namespace Tetradactyl
