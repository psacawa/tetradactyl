// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDir>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QMetaEnum>
#include <QProcessEnvironment>
#include <QRegularExpressionMatch>
#include <QSettings>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <qassert.h>

#include <qmetaobject.h>
#include <qtpreprocessorsupport.h>
#include <unistd.h>

#include "applicationtablemodel.h"
#include "common.h"
#include "utils.h"

using std::string;
using std::vector;
using Tetradactyl::BackendData;

const char createAppsTableCmd[] =
    "create table " APPS_TABLE " (id integer primary key, "
    "name text not null, "
    "path text not null, "
    "backend text not null)";

const char insertAppCmd[] =
    "insert into table " APPS_TABLE " (name, path, backend) "
    "values (?, ? , ?)";

extern vector<BackendData> backends;

Q_LOGGING_CATEGORY(lcThis, "tetradactyl.launcher.tablemodel");

namespace Tetradactyl {

ApplicationTableModel::ApplicationTableModel() {
  initDB();
  setTable(APPS_TABLE);
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
  return new ApplicationTableModel();
}

QVariant ApplicationTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= this->rowCount()) {
    return QVariant();
  }
  switch (role) {
  case Qt::DisplayRole: {
    auto nameIdx = index.siblingAtColumn(1);
    auto backendIdx = index.siblingAtColumn(3);
    return QString("%1 - %2")
        .arg(QSqlTableModel::data(nameIdx, role).toString())
        .arg(QSqlTableModel::data(backendIdx, role).toString());
  }
  default:
    return QVariant();
  }
  Q_UNREACHABLE();
}

void ApplicationTableModel::initDB() {
  QSqlQuery query;
  query.exec(createAppsTableCmd);
}

bool ApplicationTableModel::probeAndAddApp(QFileInfo file) {
  if (!file.isExecutable())
    return false;
  WidgetBackend backend = ApplicationTableModel::staticProbeExecutable(file);
  if (backend == WidgetBackend::Unknown) {
    QMessageBox::information(
        qobject_cast<QWidget *>(this->parent()),
        QString("Widget Library Unknown"),
        "Widget library could not be statically detected. This doesn't mean "
        "that Tetradactyl doesn't support it. When the application is "
        "launched, Tetradactyl will try to dynamically probe the  widget "
        "library.");
  }
  addTetradactylApp(file, backend);
  return true;
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

// allocating conversion to struct App
App *ApplicationTableModel::recordToApp(const QSqlRecord &record) {
  QMetaEnum me = QMetaEnum::fromType<WidgetBackend>();
  WidgetBackend backend = static_cast<WidgetBackend>(
      me.keyToValue(record.value("backend").toString().toLocal8Bit()));
  return new App{.name = record.value("name").toString(),
                 .file = record.value("path").toString(),
                 .backend = backend};
}

App *ApplicationTableModel::findByName(const char *name) {
  for (int row = 0; row != rowCount(); ++row) {
    QSqlRecord record = this->record(row);
    QString recordName = record.value("name").toString();
    if (recordName == name) {
      return recordToApp(record);
    }
  }
  return nullptr;
}

App *ApplicationTableModel::findByPath(const char *name) {
  for (int row = 0; row != rowCount(); ++row) {
    QSqlRecord record = this->record(row);
    QString recordName = record.value("path").toString();
    if (recordName == name) {
      return recordToApp(record);
    }
  }
  return nullptr;
}

// Blocking static probe for usin ldd in subprocess
WidgetBackend ApplicationTableModel::staticProbeExecutable(QFileInfo file) {
  QProcess lddProc;
  lddProc.start("ldd", {file.absoluteFilePath()});
  if (!lddProc.waitForFinished(-1))
    throw lddProc.error();
  QString procOutput = lddProc.readAllStandardOutput();

  for (auto backendData : backends) {
    QRegularExpression pattern(QString::fromStdString(backendData.lib));
    pattern.setPatternOptions(QRegularExpression::MultilineOption);
    if (pattern.match(procOutput).hasMatch()) {
      return backendData.type;
    }
  }
  return WidgetBackend::Unknown;
}

App ApplicationTableModel::app(const QModelIndex &index) {
  QSqlRecord record = this->record(index.row());
  QMetaEnum me = QMetaEnum::fromType<WidgetBackend>();
  WidgetBackend wb = static_cast<WidgetBackend>(
      me.keyToValue(record.value("backend").toString().toLocal8Bit()));
  qInfo() << wb;
  return App{
      .name = record.value("name").toString(),
      .file = record.value("path").toString(),
      .backend = wb,
  };
}

void ApplicationTableModel::launch(const QModelIndex &index) {
  App app = this->app(index);
  QString preloadedLib;
  app.backend == WidgetBackend::Unknown ? DYNAMIC_TETRADACTYL_LIB
                                        : backends[app.backend].lib;
  QDir launcherOrigin = QFileInfo(getLocationOfThisProgram()).dir();
  QString preloadVar =
      QString("%1/../lib/%2").arg(launcherOrigin.path()).arg(preloadedLib);
  if (setenv("LD_PRELOAD", preloadVar.toLocal8Bit().data(), 1) < 0) {
    perror("setenv");
    exit(1);
  }
  string clientProgram = app.file.toStdString();
  char *childArgv[1] = {NULL};
  execvpe(clientProgram.data(), childArgv, environ);
}

} // namespace Tetradactyl
