// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QString>
#include <QWidget>
#include <qlist.h>

#include "hint.h"

namespace Tetradactyl {

struct HintData {
  HintLabel *label;
  QPoint position;
};

class Overlay : public QWidget {
  Q_OBJECT
public:
  Overlay(QWidget *target);
  virtual ~Overlay() {}

  void addHint(QString &text, QWidget *widget);
  void removeHint(HintLabel *hint);

private:
  QList<HintData> hints;
};
} // namespace Tetradactyl
