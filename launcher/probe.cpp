// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDebug>
#include <QDir>
#include <QException>
#include <QProcess>
#include <QRegularExpression>
#include <cstdlib>
#include <exception>

#include "abi.h"
#include "common.h"
#include "launcher.h"
#include "probe.h"

using std::getenv;
using std::invalid_argument;
using std::runtime_error;

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

WidgetBackend probeBackendFromFile(QString path) {
  QFileInfo file(path);
  if (!file.exists())
    throw std::invalid_argument("file doesn't exist");

  // TODO 18/10/20 psacawa: switch on types - for now assume ELF
  return probeBackendFromElfFile(path);
}

ProbeThread::ProbeThread() : QThread() {
  QString pathEnvVar = getenv("PATH");
  binaryPaths = pathEnvVar.split(':');
}

void ProbeThread::run() {

  QRegularExpression qtWidgetsPattern("libQt.?Widgets");

  QString s("libQt6Widgets.so.6");
  auto match = qtWidgetsPattern.match(s);
  if (match.hasMatch()) {
    qInfo() << match;
  }

  for (auto path : binaryPaths) {
    qDebug() << QString("Probing %1 for Tetradactyl apps").arg(path);

    QProcess *lddProc = new QProcess;
    QDir dir(path);
    QFileInfoList dentries = dir.entryInfoList();
    for (QFileInfo file : dentries) {
      if (!file.isFile() || !file.isExecutable()) {
        continue;
      }

      // search for Qt?Widgets dependencies via ldd - not cross-platform
      lddProc->start("/bin/ldd", {file.absoluteFilePath()});
      lddProc->waitForFinished(-1);
      if (lddProc->exitCode() != 0) {
        continue;
      }

      QString out(lddProc->readAllStandardOutput());
      if (qtWidgetsPattern.match(out).hasMatch()) {
        qDebug() << lddProc->arguments();
        emit foundTetradctylApp(file, WidgetBackend::Qt6);
      }
    }
  }
}

} // namespace Tetradactyl
