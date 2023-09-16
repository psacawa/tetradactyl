// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QLayout>
#include <QString>
#include <QWidget>
#include <qlist.h>

#include "common.h"

namespace Tetradactyl {

class HintLabel;
class QWidgetActionProxy;
class OverlayLayout;

class Overlay : public QWidget {
  Q_OBJECT
public:
  Overlay(QWidget *target);
  virtual ~Overlay();

  OverlayLayout *overlayLayout();
  void addHint(QString text, QWidgetActionProxy *widgetProxy);
  void removeHint(HintLabel *hint);
  const QList<HintLabel *> &hints();
  QList<HintLabel *> visibleHints();
  void clear();
  int updateHints(QString &);
  void resetSelection(HintLabel *label = nullptr);
  void nextHint(bool forward);
  HintLabel *selectedHint();
  QWidget *selectedWidget();

private:
  QList<HintLabel *> p_hints;
  HintLabel *p_selectedHint;
};

inline const QList<HintLabel *> &Overlay::hints() { return p_hints; }

class OverlayLayout : public QLayout {
  Q_OBJECT
public:
  OverlayLayout(Overlay *overlay) : QLayout(overlay) {}
  virtual ~OverlayLayout();

  int count() const override;
  void addHint(HintLabel *hint);
  void addItem(QLayoutItem *) override;
  void setGeometry(const QRect &) override;
  QLayoutItem *itemAt(int index) const override;
  QLayoutItem *takeAt(int index) override;
  QSize sizeHint() const override;
  QSize minimumSize() const override;

private:
  Overlay *overlay() const;
  QList<QLayoutItem *> items;
};

inline OverlayLayout *Overlay::overlayLayout() {
  return static_cast<OverlayLayout *>(layout());
}
inline Overlay *OverlayLayout::overlay() const {
  return qobject_cast<Overlay *>(parentWidget());
}

} // namespace Tetradactyl
