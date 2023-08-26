#include <QDir>
#include <QLoggingCategory>
#include <QTableView>
#include <Qt>
#include <QtEnvironmentVariables>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <cstdlib>

#include "tablemodel.h"

const char createAppsTableCmd[] = "create table tetradactyl_apps ("
                                  "id integer primary key, "
                                  "name text not null, "
                                  "path text not null, "
                                  "backend text not null)";

const char insertAppCmd[] =
    "insert into table tetradactyl_apps (name, path, backend) "
    "values (?, ? , ?)";

Q_LOGGING_CATEGORY(lcThis, "tetradactyl.launcher.tablemodel");

namespace Tetradactyl {
ApplicationTableModel::ApplicationTableModel() {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  QString home = qEnvironmentVariable("HOME");
  Q_ASSERT(home != "");

  QDir xdgDataHome(qEnvironmentVariable("XDG_DATA_HOME",
                                        QString("%1/.local/share").arg(home)));
  QDir tetradactylDataHome = xdgDataHome.filePath("tetradactyl");
  if (!tetradactylDataHome.exists()) {
    if (!tetradactylDataHome.mkpath(".")) {
      qCritical() << "could not create data dir" << tetradactylDataHome;
      std::exit(1);
    }
  }

  db.setDatabaseName(tetradactylDataHome.filePath("data.sqlite"));
  qInfo() << db.databaseName();

  if (!db.open()) {
    qCritical()
        << QString("SQlite database %1 not opened").arg(db.databaseName());
  }
  db.exec(createAppsTableCmd);
}

ApplicationTableModel::~ApplicationTableModel() {}

} // namespace Tetradactyl
