// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <giomm.h>
#include <glibmm.h>

#include <QFileInfo>
#include <QObject>

#include <exception>
#include <memory>
#include <optional>

#include "app.h"
#include "probe.h"

using Gio::DesktopAppInfo;
using std::invalid_argument;
using std::optional;
using std::shared_ptr;

void __attribute__((constructor)) giommInit() {
  Glib::init();
  Gio::init();
}

namespace Tetradactyl {

void BaseApp::probe() { p_backend = probeBackendFromFile(absolutePath()); }

ExecutableFileApp::ExecutableFileApp(QString path) : p_path(path) {
  QFileInfo info(path);
  if (!info.exists())
    throw invalid_argument("path doesn't exist");
  probe();
}

QString ExecutableFileApp::name() {
  QFileInfo info(p_path);
  return info.fileName();
}

XdgDesktopApp XdgDesktopApp::fromAppId(QString appId) {
  // TODO 19/10/20 psacawa: finish this
}

XdgDesktopApp *XdgDesktopApp::fromDesktopFile(QString desktopFilePath) {
  QFileInfo info(desktopFilePath);
  if (!info.exists())
    throw invalid_argument("path doesn't exist");

  shared_ptr<DesktopAppInfo> desktopApp =
      DesktopAppInfo::create_from_filename(desktopFilePath.toStdString());
  XdgDesktopApp *app = new XdgDesktopApp;
  app->p_desktopFilePath = desktopFilePath;
  app->p_id = QString::fromStdString(desktopApp->get_id());
  app->p_commandLine = QString::fromStdString(desktopApp->get_commandline());
  app->p_executable = app->p_commandLine.split(' ').at(0);
  app->p_name = QString::fromStdString(desktopApp->get_name());
  app->p_description = QString::fromStdString(desktopApp->get_description());
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

QString XdgDesktopApp::absolutePath() {
  auto path = which(p_executable);
  if (!path)
    throw invalid_argument("no such program in PATH");
  return *path;
};

} // namespace Tetradactyl
