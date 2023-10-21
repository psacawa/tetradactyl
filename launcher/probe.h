// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QFileInfo>
#include <QThread>
#include <Qt>

#include "app.h"
#include "common.h"

namespace Tetradactyl {

WidgetBackend probeBackendFromElfFile(QString path);
WidgetBackend probeBackendFromFile(QString path);

class ProbeThread : public QThread {
  Q_OBJECT
public:
  ProbeThread();
  virtual ~ProbeThread() {}
  void run() override;
  void probeDesktopApps();
  void probeBinariesinPath();

signals:
  void foundTetradctylApp(AbstractApp *app);

private:
  QList<QString> binaryPaths;
};

} // namespace Tetradactyl
