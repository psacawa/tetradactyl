// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QFileInfo>
#include <QThread>
#include <Qt>

#include "launcher.h"

namespace Tetradactyl {

class ProbeThread : public QThread {
  Q_OBJECT
public:
  ProbeThread();
  virtual ~ProbeThread() {}
  void run() override;

signals:
  void foundTetradctylApp(QFileInfo, WidgetBackend);

private:
  QList<QString> binaryPaths;
};

} // namespace Tetradactyl
