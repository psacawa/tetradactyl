// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <Qt>

#include "app.h"
#include "common.h"

#define TETRADACTYL_APPS_DB_FILE "apps.json"

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
  int rowCount(const QModelIndex &parent) const override;

  bool contains(AbstractApp *app);

signals:
  void appDatabaseBuilt(int numNewAppsAdded);

public slots:
  void probeAndAddApp(QString file);
  void initiateBuildAppDatabase();
  void saveDatabase();
  void tryLoadDatabase();
  void resetDatabase();
  void tryAddApp(AbstractApp *app);

  void launch(const QModelIndex &index);

private:
  QList<AbstractApp *> p_apps;
};

} // namespace Tetradactyl
