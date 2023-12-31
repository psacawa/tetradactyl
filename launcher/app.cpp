// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <giomm.h>
#include <glibmm.h>

#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QObject>
#include <QStringLiteral>
#include <QtGlobal>

#include <exception>
#include <memory>
#include <optional>

#include "app.h"
#include "common.h"
#include "probe.h"
#include "utils.h"

using Gio::DesktopAppInfo;
using Glib::KeyFile;
using std::invalid_argument;
using std::optional;
using std::shared_ptr;

extern char **environ;

void __attribute__((constructor)) giommInit() {
  Glib::init();
  Gio::init();
}

namespace Tetradactyl {

// may be costly
void AbstractApp::probe() { p_backend = probeBackendFromFile(absolutePath()); }

AbstractApp *AbstractApp::fromJson(QJsonObject obj) {
  QString type = obj.value("type").toString();
  if (type == "executable") {
    return ExecutableFileApp::fromJson(obj);
  } else if (type == "desktop") {
    return XdgDesktopApp::fromJson(obj);
  } else {
    throw invalid_argument(
        qPrintable(u"Unrecognized serialized app: %1"_s.arg(type)));
  }
}

ExecutableFileApp::ExecutableFileApp(QString path,
                                     optional<WidgetBackend> backend)
    : p_path(path) {
  QFileInfo info(path);
  if (!info.exists())
    throw invalid_argument(qPrintable(u"file %1 doesn't exist"_s.arg(path)));

  if (backend.has_value()) {
    p_backend = backend.value();
  } else {
    probe();
  }
}

// ExecutableFileApp

QString ExecutableFileApp::name() const {
  QFileInfo info(p_path);
  return info.fileName();
}

QIcon ExecutableFileApp::getIcon() const {
  return QIcon(":/launcher/images/executable.png");
}

QJsonObject ExecutableFileApp::toJson() const {
  QJsonObject obj;
  obj.insert("type", "executable");
  obj.insert("filePath", absolutePath());
  obj.insert("backend", enumValueToKey(p_backend));
  return obj;
}

ExecutableFileApp *ExecutableFileApp::fromJson(QJsonObject obj) {
  QString filePath = obj.value("filePath").toString();
  WidgetBackend backend =
      enumKeyToValue<WidgetBackend>(obj.value("backend").toString());
  return new ExecutableFileApp(filePath, backend);
}

void launchAppHelper(QList<QString> argv, WidgetBackend backend = Unknown) {
  Q_ASSERT(argv.length() > 0);
  int argc = argv.length();
  QString preloadedLib;
  preloadedLib = (backend == Unknown) ? (DYNAMIC_TETRADACTYL_LIB)
                                      : backends[backend].tetradactylLib;

  QDir launcherOrigin = QFileInfo(getLocationOfThisProgram()).dir();
  QString preloadValue =
      QString("%1/../lib/%2").arg(launcherOrigin.path()).arg(preloadedLib);

  DIE_IF_NEG(setenv("LD_PRELOAD", qPrintable(preloadValue), true));

  const char *execArgv0 = qPrintable(argv.at(0));
  const char *argvCstr[argc + 1];
  for (int i = 0; i != argc; ++i)
    argvCstr[i] = qPrintable(argv.at(i));
  argvCstr[argc] = nullptr;

  execvp(execArgv0, (char *const *)argvCstr);
}

void ExecutableFileApp::launch() const {
  QList<QString> argv = {absolutePath()};
  launchAppHelper(argv, backend());
  Q_UNREACHABLE();
}

// XdgDesktopApp

XdgDesktopApp::XdgDesktopApp(QString id, QString desktopFilePath,
                             QString executable, QString name,
                             QString commandLine, QString description,
                             QString iconName)
    : p_id(id), p_desktopFilePath(desktopFilePath), p_executable(executable),
      p_name(name), p_commandLine(commandLine), p_description(description),
      p_iconName(iconName) {}

XdgDesktopApp *XdgDesktopApp::fromDesktopFile(QString desktopFilePath,
                                              bool probeExecutable) {
  QFileInfo info(desktopFilePath);
  if (!info.exists())
    throw invalid_argument(
        qPrintable(u"path %1 doesn't exist"_s.arg(desktopFilePath)));

  shared_ptr<DesktopAppInfo> desktopApp =
      DesktopAppInfo::create_from_filename(desktopFilePath.toStdString());
  XdgDesktopApp *app = new XdgDesktopApp;
  app->p_desktopFilePath = desktopFilePath;
  app->p_id = QString::fromStdString(desktopApp->get_id());
  app->p_commandLine = QString::fromStdString(desktopApp->get_commandline());
  app->p_executable = app->p_commandLine.split(' ').at(0);
  app->p_name = QString::fromStdString(desktopApp->get_name());
  app->p_description = QString::fromStdString(desktopApp->get_description());

  // simpler way to get the icon name?
  shared_ptr<KeyFile> keyFile = KeyFile::create();
  keyFile->load_from_file(desktopApp->get_filename());
  string iconName = "";
  try {
    iconName = keyFile->get_string("Desktop Entry", "Icon");
  } catch (const Glib::KeyFileError &e) {
    // TODO 21/10/20 psacawa: no icon entry, choose default
  }
  app->p_iconName = QString::fromStdString(iconName);
  app->p_icon = QIcon::fromTheme(app->p_iconName);

  if (probeExecutable)
    app->probe();

  return app;
}

static optional<QString> which(QString prog) {
  QList<QByteArray> paths = qgetenv("PATH").split(':');
  for (auto path : paths) {
    QFileInfo info(QString("%1/%2").arg(path, prog));
    if (info.exists())
      return info.filePath();
  }
  return optional<QString>();
}

QString XdgDesktopApp::absolutePath() const {
  if (p_executable.front() == '/') {
    // absolute path
    return p_executable;
  }
  auto path = which(p_executable);
  if (!path)
    throw invalid_argument(
        qPrintable(QString("no program %1 in PATH").arg(p_executable)));
  return *path;
};

QJsonObject XdgDesktopApp::toJson() const {
  QJsonObject obj;
  obj.insert("type", "desktop");
  obj.insert("desktopFilePath", desktopPath());
  obj.insert("backend", enumValueToKey(p_backend));
  return obj;
}

XdgDesktopApp *XdgDesktopApp::fromJson(QJsonObject obj) {
  QString desktopFilePath = obj.value("desktopFilePath").toString();
  WidgetBackend backend =
      enumKeyToValue<WidgetBackend>(obj.value("backend").toString());
  // to save time in loading, we parse the desktop file, but don't probe
  // if this becomes still to much, we will avoid parsing .desktop file
  // altogether
  XdgDesktopApp *app = XdgDesktopApp::fromDesktopFile(desktopFilePath, false);
  app->setBackend(backend);
  return app;
}

QIcon XdgDesktopApp::getIcon() const { return p_icon; }

void XdgDesktopApp::launch() const {
  auto argv = {commandLine().split(" ").at(0)};
  launchAppHelper(argv, backend());
}

} // namespace Tetradactyl
