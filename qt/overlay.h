// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QLabel>
#include <QLayout>
#include <QString>
#include <QWidget>
#include <qlist.h>

#include "common.h"

namespace Tetradactyl {

class HintLabel;
class QWidgetActionProxy;
class OverlayLayout;
class Overlay;
class WindowController;

QList<HintLabel *> findHintsByTargetHelper(Overlay *overlay,
                                           const QMetaObject *mo);

class Overlay : public QWidget {
  Q_OBJECT
public:
  Overlay(WindowController *controller, QWidget *target, bool isMain = true);
  virtual ~Overlay();

  OverlayLayout *overlayLayout();
  void addHint(QString text, QWidgetActionProxy *widgetProxy);
  void removeHint(HintLabel *hint);
  const QList<HintLabel *> &hints();
  const QLabel *statusIndicator();
  QList<HintLabel *> visibleHints();
  void clear();
  int updateHints(QString &);
  void resetSelection(HintLabel *label = nullptr);
  void nextHint(bool forward);
  HintLabel *selectedHint();
  QWidget *selectedWidget();

  template <typename T> inline QList<HintLabel *> findHintsByTarget() {
    typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type
        ObjType;
    return findHintsByTargetHelper(this, &ObjType::staticMetaObject);
  }

private:
  QList<HintLabel *> p_hints;
  HintLabel *p_selectedHint;
  QLabel *p_statusIndicator;
  WindowController *controller;

  friend class OverlayLayout;
};

inline const QList<HintLabel *> &Overlay::hints() { return p_hints; }
inline const QLabel *Overlay::statusIndicator() { return p_statusIndicator; }

class OverlayLayout : public QLayout {
  Q_OBJECT
public:
  OverlayLayout(Overlay *overlay);
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
  QLayoutItem *statusIndicatorItem;
  QList<QLayoutItem *> items;
};

inline OverlayLayout *Overlay::overlayLayout() {
  return static_cast<OverlayLayout *>(layout());
}
inline Overlay *OverlayLayout::overlay() const {
  return qobject_cast<Overlay *>(parentWidget());
}

} // namespace Tetradactyl
