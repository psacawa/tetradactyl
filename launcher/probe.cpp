// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <giomm.h>
#include <glibmm.h>

#include <QDebug>
#include <QDir>
#include <QException>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>

#include <cstdlib>
#include <exception>
#include <memory>

#include "abi.h"
#include "common.h"
#include "launcher.h"
#include "probe.h"

using Gio::AppInfo;
using Gio::DesktopAppInfo;
using std::exception;
using std::getenv;
using std::invalid_argument;
using std::runtime_error;
using std::shared_ptr;

// Probing an  executable to determine whether it uses a supported backend can
// proceed in multiple ways:
// - The file can be a binary with an explicit dynamic link dependency on the
// library with the appropriate name. The dependency can be detected statically
// with a tool like ldd.
// - The file can be a native binary that dynamically (as in dlopen) loads the
// library. E.g. vlc uses this.
// - The file may be an readable or binary executable for some runtime such as
// JRE or Python. In some cases, these may be *might* determined statically. For
// instance, java .jars have builtin dependency data that  can be read with a
// CLI tool like jdeps. However, the user-facing executable is on POSIX a  shell
// script which execs the java interpreter with the appropriate .jar. So, do
// statically detect the backend, one would need to grep the script to find the
// jar file, then statically analyze it. Other runtime environments, e.g. Python
// are even more difficult because of their dynamic nature.

namespace Tetradactyl {

WidgetBackend probeBackendFromElfFile(QString path) {
  QProcess lddProc;
  lddProc.startCommand(QString("/bin/ldd %1").arg(path));
  int finished = lddProc.waitForFinished(1000);
  if (!finished || lddProc.exitStatus() != QProcess::NormalExit)
    throw runtime_error("ldd didn't finish normally");
  if (lddProc.exitCode() != 0) {
    qWarning() << path << "isn't dynamic executable according to ldd";
    return WidgetBackend::Unknown;
  }
  QByteArray lddOut = lddProc.readAllStandardOutput();
  QList<QByteArray> lines = lddOut.split('\n');
  for (auto backendData : backends) {
    QRegularExpression libPattern(backendData.lib);
    for (auto line : lines) {
      auto match = libPattern.match(line);
      if (match.hasMatch())
        return backendData.type;
    }
  }
  return WidgetBackend::Unknown;
}

// identify ELF file via magic header \x7f\x45\x4c\x46
static bool isElfExecutable(QFileInfo file) {
  if (!file.isExecutable())
    return false;
  QFile fd(file.filePath());
  fd.open(QFile::ReadOnly);
  QByteArray prefix;
  prefix.reserve(4);
  int numRead = fd.read(prefix.data(), 4);
  return numRead == 4 && prefix == "\x7f\x45\x4c\x46";
}

WidgetBackend probeBackendFromFile(QString path) {
  QFileInfo file(path);
  if (!file.exists())
    throw std::invalid_argument("file doesn't exist");

  if (!isElfExecutable(file))
    return Unknown;

  // switch on types? - for now only ELF supported
  return probeBackendFromElfFile(path);
}

ProbeThread::ProbeThread() : QThread() {
  QString pathEnvVar = getenv("PATH");
  binaryPaths = pathEnvVar.split(':');
}

void ProbeThread::run() {
  probeDesktopApps();
  probeBinariesinPath();
}

void ProbeThread::probeDesktopApps() {
  for (auto appInfo : AppInfo::get_all()) {
    shared_ptr<DesktopAppInfo> desktopApp =
        std::static_pointer_cast<DesktopAppInfo>(appInfo);
    QString filePath = QString::fromStdString(desktopApp->get_filename());
    AbstractApp *app;
    try {
      app = XdgDesktopApp::fromDesktopFile(filePath);
    } catch (exception &e) {
      qWarning() << e.what();
      continue;
    }
    if (app->backend() == Unknown)
      continue;

    emit foundTetradctylApp(app);
  }
}

void ProbeThread::probeBinariesinPath() {
  for (auto path : binaryPaths) {
    qDebug() << QString("Probing %1 for Tetradactyl apps").arg(path);

    QProcess *lddProc = new QProcess;
    QDir dir(path);
    QFileInfoList dentries = dir.entryInfoList();
    for (QFileInfo file : dentries) {
      if (!file.isFile() || !file.isExecutable())
        continue;

      AbstractApp *app = new ExecutableFileApp(file.filePath());

      if (app->backend() == Unknown)
        continue;

      emit foundTetradctylApp(app);
    }
  }
}

} // namespace Tetradactyl
