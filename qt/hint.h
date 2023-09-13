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

  HintLabel(QString label, QWidget *target, Overlay *parent);

  inline bool isSelected();
  void setSelected(bool);

  void paintEvent(QPaintEvent *) override;

  QWidget *target;

private:
  bool selected;
};

inline bool HintLabel::isSelected() { return selected; }

} // namespace Tetradactyl
