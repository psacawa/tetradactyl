// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractListModel>
#include <QDir>
#include <QFileIconProvider>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLoggingCategory>

#include <exception>

#include "app.h"
#include "applicationmodel.h"
#include "probe.h"

using std::exception;
using std::invalid_argument;
using std::runtime_error;
using Tetradactyl::BackendData;

Q_LOGGING_CATEGORY(lcThis, "tetradactyl.launcher.appmodel");

QFileIconProvider iconProvider;

namespace Tetradactyl {

ApplicationModel::ApplicationModel(QObject *parent)
    : QAbstractListModel(parent) {}

int ApplicationModel::rowCount(const QModelIndex &parent) const {
  return p_apps.count();
}

QVariant ApplicationModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= p_apps.length())
    return {};
  AbstractApp *app = p_apps.at(index.row());
  switch (role) {
  case Qt::DisplayRole:
    return app->name();
  case Qt::DecorationRole:
    // return QIcon::fromTheme("anki");
    return app->getIcon();
  default:
    return {};
  }
}

bool ApplicationModel::contains(AbstractApp *app) {
  const QMetaObject *mo = app->metaObject();
  for (auto otherApp : p_apps) {
    if (*app == *otherApp) {
      return true;
    }
  }
  return false;
}

void ApplicationModel::probeAndAddApp(QString filePath) {
  QFileInfo info(filePath);
  if (!info.exists())
    throw invalid_argument(
        qUtf8Printable(QString("File %1 doesn't exist").arg(filePath)));
  AbstractApp *newApp;
  if (info.isExecutable()) {
    newApp = new ExecutableFileApp(filePath);
  } else if (info.fileName().split(".").last() == "desktop") {
    newApp = XdgDesktopApp::fromDesktopFile(filePath);
  } else
    throw invalid_argument("File type not recognized");

  tryAddApp(newApp);
}

void ApplicationModel::initiateBuildAppDatabase() {
  ProbeThread *thread = new ProbeThread;
  thread->start();

  // This can't be stored on the stack, OTOH dynamic storage couples things in
  // an unfortunate way.
  int *numNewAppsAdded = new int;
  *numNewAppsAdded = 0;

  connect(thread, &ProbeThread::foundTetradctylApp, this,
          [this, numNewAppsAdded](AbstractApp *app) {
            try {
              tryAddApp(app);
              *numNewAppsAdded += 1;
            } catch (exception &e) {
              qWarning() << e.what();
            }
          });
  QObject::connect(thread, &QThread::finished, this,
                   [this, thread, numNewAppsAdded] {
                     delete thread;
                     emit appDatabaseBuilt(*numNewAppsAdded);
                     saveDatabase();
                     delete numNewAppsAdded;
                   });
}

static QDir tetradactylDataDir() {
  QString home = qEnvironmentVariable("HOME");
  if (home == "")
    throw std::runtime_error("no HOME environmental variable");

  QDir dataDir(qEnvironmentVariable("XDG_DATA_HOME",
                                    QString("%1/.local/share").arg(home)));
  dataDir.mkdir("tetradactyl");
  dataDir.cd("tetradactyl");
  return dataDir;
}

static QFile appDatabaseFile() {
  QDir dataDir = tetradactylDataDir();
  QString filePath = dataDir.path() + "/" TETRADACTYL_APPS_DB_FILE;
  return filePath;
}

void ApplicationModel::saveDatabase() {
  QJsonArray array;
  QJsonObject object;
  for (auto app : p_apps) {
    array.append(app->toJson());
  }
  object["applications"] = array;

  QFile appDBFile = appDatabaseFile();
  QJsonDocument document;
  document.setObject(object);
  appDBFile.open(QFile::WriteOnly);
  appDBFile.write(document.toJson());
  appDBFile.close();
}

void ApplicationModel::tryLoadDatabase() {
  QFile appDBFile = appDatabaseFile();
  appDBFile.open(QFile::ReadOnly);
  if (!appDBFile.isOpen())
    return;

  QByteArray json = appDBFile.readAll();
  appDBFile.close();

  QJsonParseError jsonErr;
  QJsonDocument document = QJsonDocument::fromJson(json, &jsonErr);
  if (jsonErr.error != QJsonParseError::NoError)
    throw runtime_error(qUtf8Printable(jsonErr.errorString()));

  // better solution for schema validation?
  if (!document.isObject())
    throw runtime_error("Bad " TETRADACTYL_APPS_DB_FILE);
  QJsonObject obj = document.object();

  if (!obj.contains("applications"))
    throw runtime_error("Bad " TETRADACTYL_APPS_DB_FILE);
  QJsonValue apps = obj.value("applications");

  if (!apps.isArray())
    throw runtime_error("Bad " TETRADACTYL_APPS_DB_FILE);
  QJsonArray appsArray = apps.toArray();

  for (auto jsonApp : appsArray) {
    QJsonObject jsonAppObj = jsonApp.toObject();
    tryAddApp(AbstractApp::fromJson(jsonAppObj));
  }
}

void ApplicationModel::resetDatabase() {
  QFile dbFile = appDatabaseFile();
  if (dbFile.exists()) {
    dbFile.remove();
  }
}

void ApplicationModel::tryAddApp(AbstractApp *app) {
  if (contains(app))
    throw AppAlreadyPresent(qUtf8Printable(app->name()));
  beginResetModel();
  p_apps.append(app);
  endResetModel();
}

void ApplicationModel::launch(const QModelIndex &index) {
  p_apps.at(index.row())->launch();
}

} // namespace Tetradactyl
