#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QTableView>
#include <Qt>
#include <QtGlobal>

#include <cstdlib>
#include <qfileinfo.h>
#include <string>
#include <vector>

#include "applicationtablemodel.h"
#include "common.h"
#include "ui_launcherwindow.h"

using std::string;
using std::vector;

namespace Tetradactyl {

class ApplicationTableModel;

class Launcher : public QMainWindow {
  Q_OBJECT
public:
  Launcher();
  virtual ~Launcher() {}
private slots:

  void on_applicationView_activated(const QModelIndex &index);
  void
  on_applicationView_doubleClicked(const QModelIndex &index = QModelIndex());
  void on_launchButton_clicked(bool clicked);

private:
  ApplicationTableModel *model;

  void fixupUi();

  Ui::LauncherWindow *ui;
};
} // namespace Tetradactyl
