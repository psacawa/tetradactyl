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
  inline void setSelected(bool);

private:
  bool selected;
};

inline bool HintLabel::getSelected() { return selected; }
inline void HintLabel::setSelected(bool _selected) { selected = _selected; }

} // namespace Tetradactyl
