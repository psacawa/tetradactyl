// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QLabel>
#include <QString>

namespace Tetradactyl {

class Overlay;

class QWidgetActionProxy;

class HintLabel : public QLabel {
  Q_OBJECT
public:
  Q_PROPERTY(bool selected READ isSelected WRITE setSelected);
  Q_PROPERTY(QPoint p_positionInTarget READ positionInTarget);
  Q_PROPERTY(QPoint p_positionInOverlay READ positionInOverlay);

  HintLabel(QString label, QWidget *target, Overlay *parent,
            QWidgetActionProxy *proxy);
  virtual ~HintLabel();

  inline bool isSelected();
  void setSelected(bool);
  QPoint positionInTarget();
  QPoint positionInOverlay();

  void paintEvent(QPaintEvent *) override;

  QWidgetActionProxy *proxy;
  QPoint p_positionInOverlay;
  QWidget *target;

private:
  bool selected;
  QPoint p_positionInTarget;
};

inline bool HintLabel::isSelected() { return selected; }
inline QPoint HintLabel::positionInTarget() { return p_positionInTarget; }
inline QPoint HintLabel::positionInOverlay() { return p_positionInOverlay; }

} // namespace Tetradactyl
