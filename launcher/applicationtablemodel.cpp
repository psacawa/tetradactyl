#include <QDir>
#include <QLoggingCategory>
#include <QMetaEnum>
#include <QMetaType>
#include <QProcessEnvironment>
#include <Qt>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

#include <cstdlib>

#include "applicationtablemodel.h"

const char createAppsTableCmd[] =
    "create table " APPS_TABLE " (id integer primary key, "
    "name text not null, "
    "path text not null, "
    "backend text not null)";

const char insertAppCmd[] =
    "insert into table " APPS_TABLE " (name, path, backend) "
    "values (?, ? , ?)";

Q_LOGGING_CATEGORY(lcThis, "tetradactyl.launcher.tablemodel");

namespace Tetradactyl {

ApplicationTableModel::ApplicationTableModel() {

  setTable(APPS_TABLE);
  qInfo() << lastError();
  if (!select()) {
    qInfo() << "select" << lastError();
  }
}

ApplicationTableModel::~ApplicationTableModel() {}

// Factory for ApplicationTableModel ensures that a QSqlDatabase has be
// configured and and database tables have be created before
// ApplicationTableModel is constructed. Expectations of QSqlTableModel mean
// that this can't be done in ApplicationTableModel::ApplicationTableModel()
ApplicationTableModel *ApplicationTableModel::createApplicationTableModel() {
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
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(tetradactylDataHome.filePath("data.sqlite"));

  if (!db.open()) {
    qCritical()
        << QString("SQlite database %1 not opened").arg(db.databaseName());
  }
  qInfo() << db.tables();
  return new ApplicationTableModel();
}

void ApplicationTableModel::initDB() {
  QSqlQuery query;
  query.exec(createAppsTableCmd);
}

void ApplicationTableModel::addTetradactylApp(QFileInfo file,
                                              WidgetBackend backend) {

  QSqlRecord newRecord;

  QString backendString =
      QMetaEnum::fromType<WidgetBackend>().valueToKey(backend);
  qWarning() << backendString;

  QSqlField nameField("name", QMetaType(QMetaType::QString));
  QSqlField pathField("path", QMetaType(QMetaType::QString));
  QSqlField backendField("backend", QMetaType(QMetaType::QString));

  nameField.setValue(file.baseName());
  pathField.setValue(file.absoluteFilePath());
  backendField.setValue(backendString);

  newRecord.append(nameField);
  newRecord.append(pathField);
  newRecord.append(backendField);

  qWarning() << newRecord;

  /*
   *   newRecord.setValue(1, file.baseName());
   *   newRecord.setValue(2, file.absoluteFilePath());
   *   newRecord.setValue(3, backendString);
   *
   */
  if (!this->insertRecord(-1, newRecord)) {
    qCritical() << "record not added:" << newRecord;
  }
}

void ApplicationTableModel::launch(const QModelIndex &index) {
  /*
   * App *app = apps[index.row()];

   * qInfo() << QString("Launching %1").arg(app->file.absoluteFilePath());
   * QProcessEnvironment childEnv(QProcessEnvironment::InheritFromParent);
   * if (app->backend == WidgetBackend::Qt6) {
   *   childEnv.insert("LD_PRELOAD", "libtetradactyl-qt.so");
   * }
   * QProcess tetradactylProcess;
   * tetradactylProcess.setProcessEnvironment(childEnv);
   * tetradactylProcess.start(app->file.absoluteFilePath());
   * tetradactylProcess.waitForFinished(-1);
   */
  // TODO 15/08/20 psacawa: pass args
}

} // namespace Tetradactyl
