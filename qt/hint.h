// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QLabel>
#include <QString>

#include "overlay.h"

namespace Tetradactyl {
class HintLabel : public QLabel {
  Q_OBJECT
public:
  Q_PROPERTY(bool selected READ isSelected WRITE setSelected);

  HintLabel(QString label, QWidget *target, Overlay *parent,
            QPoint positionInTarget = QPoint(0, 0));

  inline bool isSelected();
  void setSelected(bool);

  void paintEvent(QPaintEvent *) override;

  QPoint positionInOverlay;
  QWidget *target;

private:
  bool selected;
  QPoint positionInTarget;
};

inline bool HintLabel::isSelected() { return selected; }

} // namespace Tetradactyl
