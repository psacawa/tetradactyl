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

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;

public slots:
  bool probeAndAddApp(QFileInfo file);
  void addTetradactylApp(QFileInfo file,
                         WidgetBackend backend = WidgetBackend::Unknown);
  void launch(const QModelIndex &index);

private:
  void initDB();
  static WidgetBackend staticProbeExecutable(QFileInfo file);

  QSqlDatabase *db;

  friend class ApplicationTableModelTest;
};

} // namespace Tetradactyl
