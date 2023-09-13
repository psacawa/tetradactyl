// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QString>
#include <QWidget>
#include <qlist.h>

namespace Tetradactyl {

class HintLabel;

class Overlay : public QWidget {
  Q_OBJECT
public:
  Overlay(QWidget *target);
  virtual ~Overlay();

  void addHint(QString text, QWidget *widget);
  void removeHint(HintLabel *hint);
  const QList<HintLabel *> &hints();
  QList<HintLabel *> visibleHints();
  void clear();
  int updateHints(QString &);
  void resetSelection(HintLabel *label = nullptr);
  void nextHint(bool forward);
  HintLabel *selectedHint();
  QWidget *selectedWidget();
  void paintEvent(QPaintEvent *event) override;

private:
  QList<HintLabel *> p_hints;
  HintLabel *p_selectedHint;
};

inline const QList<HintLabel *> &Overlay::hints() { return p_hints; }

} // namespace Tetradactyl
