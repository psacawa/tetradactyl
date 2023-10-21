// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QAbstractListModel>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <Qt>
#include <QtGlobal>

#include <cstdlib>
#include <qfileinfo.h>
#include <string>
#include <vector>

#include "common.h"

using std::string;
using std::vector;

namespace Ui {
class LauncherWindow;
} // namespace Ui

namespace Tetradactyl {

class ApplicationModel;

class Launcher : public QMainWindow {
  Q_OBJECT
public:
  Launcher();
  virtual ~Launcher() {}

private slots:
  void onApplicationViewActivated(const QModelIndex &index);
  void onLaunchButtonClicked(bool clicked);
  void onAddButtonClicked(bool clicked);
  void reportScanResults(int numNewAppsAdded);
  void onSearchTextChanged(const QString &text);

private:
  ApplicationModel *sourceModel;
  QSortFilterProxyModel *displayModel;

  void fixupUi();

  Ui::LauncherWindow *ui;
};
} // namespace Tetradactyl
