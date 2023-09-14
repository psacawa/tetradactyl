// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QModelIndex>
#include <QPoint>
#include <QWidget>

namespace Tetradactyl {

struct HintData {
  QWidget *widget;
  QPoint point = QPoint(0, 0);
};

} // namespace Tetradactyl
