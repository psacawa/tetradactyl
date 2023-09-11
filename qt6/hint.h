// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QLabel>
#include <QString>

namespace Tetradactyl {
class HintLabel : public QLabel {
  Q_OBJECT
public:
  Q_PROPERTY(bool selected READ getSelected WRITE setSelected);

  HintLabel(QString label, QWidget *target);

  inline bool getSelected();
  void setSelected(bool);

  QWidget *target;

private:
  bool selected;
};

inline bool HintLabel::getSelected() { return selected; }

} // namespace Tetradactyl
