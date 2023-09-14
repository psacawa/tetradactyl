// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QLabel>
#include <QString>

#include "overlay.h"

namespace Tetradactyl {

class QWidgetActionProxy;

class HintLabel : public QLabel {
  Q_OBJECT
public:
  Q_PROPERTY(bool selected READ isSelected WRITE setSelected);
  Q_PROPERTY(QPoint positionInTarget READ getPositionInTarget);

  HintLabel(QString label, QWidget *target, Overlay *parent,
            QWidgetActionProxy *proxy);
  virtual ~HintLabel();

  inline bool isSelected();
  void setSelected(bool);
  QPoint getPositionInTarget();

  void paintEvent(QPaintEvent *) override;

  QWidgetActionProxy *proxy;
  QPoint positionInOverlay;
  QWidget *target;

private:
  bool selected;
  QPoint positionInTarget;
};

inline bool HintLabel::isSelected() { return selected; }
inline QPoint HintLabel::getPositionInTarget() { return positionInTarget; }

} // namespace Tetradactyl
