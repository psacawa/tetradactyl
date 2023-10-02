// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QLabel>
#include <QLayout>
#include <QString>
#include <QWidget>
#include <qlist.h>

#include "common.h"

namespace Tetradactyl {

class CommandLine;
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
  Q_PROPERTY(QList<HintLabel *> hints READ hints);
  Q_PROPERTY(QList<HintLabel *> visibleHints READ visibleHints);
  Q_PROPERTY(const QLabel *statusIndicator READ statusIndicator CONSTANT);
  Q_PROPERTY(HintLabel *selectedHint READ selectedHint);
  Q_PROPERTY(QWidget *selectedWidget READ selectedWidget);
  Q_PROPERTY(CommandLine *commandLine READ commandLine CONSTANT);
  Q_PROPERTY(WindowController *windowController MEMBER controller);

  Overlay(WindowController *controller, QWidget *target, bool isMain = true);
  virtual ~Overlay();

  OverlayLayout *overlayLayout();

  const QList<HintLabel *> &hints();
  const QLabel *statusIndicator();
  CommandLine *commandLine();
  QList<HintLabel *> visibleHints();
  HintLabel *selectedHint();
  QWidget *selectedWidget();

public slots:
  void addHint(QString text, QWidgetActionProxy *widgetProxy);
  void popHint(HintLabel *hint);
  void clear();
  int updateHints(QString &);
  void resetSelection(HintLabel *label = nullptr);
  void nextHint(bool forward);

public:
  template <typename T> inline QList<HintLabel *> findHintsByTarget() {
    using ObjType = std::remove_cv_t<std::remove_pointer_t<T>>;
    return findHintsByTargetHelper(this, &ObjType::staticMetaObject);
  }

private:
  WindowController *controller;
  QList<HintLabel *> p_hints;
  HintLabel *p_selectedHint;
  QLabel *p_statusIndicator;
  CommandLine *p_commandLine;

  friend class OverlayLayout;

  friend QDebug operator<<(QDebug debug, const Overlay *overlay);
};

inline const QList<HintLabel *> &Overlay::hints() { return p_hints; }
inline const QLabel *Overlay::statusIndicator() { return p_statusIndicator; }
inline CommandLine *Overlay::commandLine() { return p_commandLine; }

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
  QLayoutItem *commandLineItem;
  QList<QLayoutItem *> items;
};

inline OverlayLayout *Overlay::overlayLayout() {
  return static_cast<OverlayLayout *>(layout());
}
inline Overlay *OverlayLayout::overlay() const {
  return qobject_cast<Overlay *>(parentWidget());
}

} // namespace Tetradactyl
