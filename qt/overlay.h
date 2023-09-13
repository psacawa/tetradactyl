// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QString>
#include <QWidget>
#include <qlist.h>

namespace Tetradactyl {

class HintLabel;

struct HintData {
  HintLabel *label;
  QPoint position;
};

class Overlay : public QWidget {
  Q_OBJECT
public:
  Overlay(QWidget *target);
  virtual ~Overlay() {}

  void addHint(QString text, QWidget *widget);
  void removeHint(HintLabel *hint);
  QList<HintLabel *> hints();
  QList<HintLabel *> visibleHints();
  void clear();
  int updateHints(QString &);
  void resetSelection(HintLabel *label = nullptr);
  void nextHint(bool forward);
  HintLabel *selectedHint();
  QWidget *selectedWidget();
  void paintEvent(QPaintEvent *event) override;

private:
  QList<HintData> hintData;
  HintLabel *p_selectedHint;
};
} // namespace Tetradactyl
