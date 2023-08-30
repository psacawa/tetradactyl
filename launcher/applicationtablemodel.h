#pragma once

#include <QAbstractTableModel>
#include <QFileInfo>
#include <Qt>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>

#include "common.h"

#define APPS_TABLE "tetradactyl_apps"

namespace Tetradactyl {

class ApplicationTableModel : public QSqlTableModel {
  Q_OBJECT
public:
  ApplicationTableModel();
  virtual ~ApplicationTableModel();
  static ApplicationTableModel *createApplicationTableModel();

public slots:
  void addTetradactylApp(QFileInfo file,
                         WidgetBackend backend = WidgetBackend::None);
  void launch(const QModelIndex &index);

private:
  void initDB();

  QSqlDatabase *db;

  friend class ApplicationTableModelTest;
};

} // namespace Tetradactyl
