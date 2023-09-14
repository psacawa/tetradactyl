// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QAbstractItemView>
#include <QList>
#include <QMetaObject>
#include <QStackedWidget>
#include <QTabBar>
#include <QWidget>

#include <map>
#include <qabstractbutton.h>
#include <qassert.h>
#include <qobject.h>

// c++ forbids  forward reference to enum HintMode
#include "actionmacros.h"
#include "controller.h"

using std::map;

namespace Tetradactyl {

struct HintData {
  QWidget *widget;
  QPoint point = QPoint(0, 0);
};

// Facilitates multi-stage actions, such as menu navigation. Also hold context
// that informs how the hinting procedure proceeds.
class BaseAction : public QObject {
  Q_OBJECT
public:
  HintMode mode;
  WindowController *winController;
  virtual void act();
  BaseAction(WindowController *controller) : winController(controller) {}
  virtual ~BaseAction() {}
  virtual void accept(QWidget *widget);
  virtual bool done() = 0;

  static BaseAction *createActionByHintMode(HintMode, WindowController *);
};

class ActivateAction : public BaseAction {
  Q_OBJECT
public:
  ActivateAction(WindowController *controller);
  virtual ~ActivateAction() {}
  virtual bool done() override;
};

class EditAction : public BaseAction {
  Q_OBJECT
public:
  EditAction(WindowController *controller);
  virtual ~EditAction() {}
  virtual bool done() override;
};

class FocusAction : public BaseAction {
  Q_OBJECT
public:
  FocusAction(WindowController *controller);
  virtual ~FocusAction() {}
  virtual bool done() override;
};

class YankAction : public BaseAction {
  Q_OBJECT
public:
  YankAction(WindowController *controller);
  virtual ~YankAction() {}
  virtual bool done() override;
};

class ContextMenuAction : public BaseAction {
  Q_OBJECT
public:
  ContextMenuAction(WindowController *controller);
  virtual ~ContextMenuAction() {}
};

extern map<HintMode, BaseAction *> actionRegistry;

// Widget Proxies

// A speculative idea: The action-specific code is accompanoed by an inheritance
// hierarchy of "widget proxy" classse that mirrors the QWidget inheritance
// hierarchy. These tell whether/how to implement the routines in a manner
// specific to the widget, while still permitting code reuse via the
// inheritance. Context is passed to the actor methods via the actions, which
// have a context attached to them.

// When visiting a widget, you may or may not want to hint it. Separately, you
// may or may not want to recurse under it to search for more hints.
// Notwithstanding some edge cases, typically a hinted recursed won't be
// recursed under. The methods is*able() determine whether the widget itself
// should generate a hint. The caller does the adding. Conversely, the methods
// hint*able For now, entering is*able and hint*able methods means that the
// widget is enabled and visible: hint*able is responsible for calling
// visible(QWidget*) for the children it wishes to recurse to.

// Therefore, according to current *doctrine*, e.g. a QTabBar would handle
// hintable tabs and recurse to the underlying QStackedWdiget in
// hintActivatable. Itself, it is not activatable. Likewise, QMenu handles it's
// options in hintActivatable. NB this holds whether or not the hintable
// "subwidgets" are real widgets in the true sense, or rather are pseudo-widgets
// implemented by QStyle::drawControl etc. (short-sighthed?) This doctrine lasts
// until the next surprise.

// In actionmacros.h we define preprocessor macros to cut  down on boilerplate
// code for each QWidget subclass.

//
// QWidgetActionProxy
//

class QWidgetActionProxy {
public:
  QWidgetActionProxy() {}
  virtual ~QWidgetActionProxy() {}

  inline static bool visible(QWidget *w);

  // can the widget itself be hinted/actioned?
  virtual bool isActivatable(ActivateAction *action, QWidget *widget) {
    return false;
  }
  virtual bool isYankable(YankAction *action, QWidget *widget) { return false; }
  virtual bool isEditable(EditAction *action, QWidget *widget) { return false; }
  virtual bool isFocusable(FocusAction *action, QWidget *widget) {
    return false;
  }
  virtual bool isContextMenuable(ContextMenuAction *action, QWidget *widget);

  // how to recurse the hinting under the
  // widget?
  virtual void hintGeneric(BaseAction *action, QWidget *widget,
                           QList<HintData> &ret);
  virtual void hintActivatable(ActivateAction *action, QWidget *widget,
                               QList<HintData> &ret);
  virtual void hintYankable(YankAction *action, QWidget *widget,
                            QList<HintData> &ret);
  virtual void hintEditable(EditAction *action, QWidget *widget,
                            QList<HintData> &ret);
  virtual void hintFocusable(FocusAction *action, QWidget *widget,
                             QList<HintData> &ret);
  virtual void hintContextMenuable(ContextMenuAction *action, QWidget *widget,
                                   QList<HintData> &ret) {
    return;
  }

  // how to action widget type?
  bool actGeneric(BaseAction *, QWidget *widget);
  virtual bool activate(ActivateAction *action, QWidget *widget) {
    return false;
  }
  virtual bool yank(YankAction *action, QWidget *widget) { return false; }
  virtual bool edit(EditAction *action, QWidget *widget) { return false; }
  virtual bool focus(FocusAction *action, QWidget *widget) {
    widget->setFocus();
    return true;
  }
  virtual bool contextMenu(ContextMenuAction *action, QWidget *widget) {
    return false;
  }

  static QWidgetActionProxy *getForMetaObject(const QMetaObject *mo);

private:
  static map<const QMetaObject *, QWidgetActionProxy *> registry;
};

inline bool QWidgetActionProxy::visible(QWidget *w) {
  return w->isVisible() && w->isEnabled();
}

// QAbstractButtonActionProxy

class QAbstractButtonActionProxy : public QWidgetActionProxy {
public:
  QAbstractButtonActionProxy() {}
  virtual ~QAbstractButtonActionProxy() {}

  ACTIONPROXY_TRUE_SELF_ACTIVATABLE_DEF
  ACTIONPROXY_TRUE_SELF_YANKABLE_DEF
  ACTIONPROXY_TRUE_SELF_FOCUSABLE_DEF

  ACTIONPROXY_NULL_RECURSE_DEF

  virtual bool activate(ActivateAction *action, QWidget *widget) override;
  virtual bool focus(FocusAction *action, QWidget *widget) override;
  virtual bool yank(YankAction *action, QWidget *widget) override;
};

// QAbstractItemViewActionProxy

class QAbstractItemViewActionProxy : public QWidget {
public:
  QAbstractItemViewActionProxy() {}
  virtual ~QAbstractItemViewActionProxy() {}

private:
  /* data */
};

// QGroupBoxActionProxy

class QGroupBoxActionProxy : public QWidgetActionProxy {
public:
  virtual bool isActivatable(ActivateAction *action, QWidget *widget) override;
  virtual bool activate(ActivateAction *action, QWidget *widget) override;
};

// QLabelActionProxy

class QLabelActionProxy : public QWidgetActionProxy {
public:
  ACTIONPROXY_TRUE_SELF_YANKABLE_DEF
  ACTIONPROXY_NULL_RECURSE_DEF
  virtual bool yank(YankAction *action, QWidget *widget) override;
};

// QLineEditActionProxy

class QLineEditActionProxy : public QWidgetActionProxy {
public:
  ACTIONPROXY_TRUE_SELF_EDITABLE_DEF
  ACTIONPROXY_TRUE_SELF_FOCUSABLE_DEF

  ACTIONPROXY_DEFAULT_ACTION_EDITABLE_DEF
};

// QTabBarActionProxy

class QTabBarActionProxy : public QWidgetActionProxy {
public:
  QTabBarActionProxy() {}
  virtual ~QTabBarActionProxy() {}

  void hintActivatable(ActivateAction *action, QWidget *widget,
                       QList<HintData> &ret) override;

  bool activate(ActivateAction *action, QWidget *widget) override;

private:
  QList<QPoint> probeTabLocations(QTabBar *bar);
  int tabIndex;
};

// QStackedWidgetActionProxy

class QStackedWidgetActionProxy : public QWidgetActionProxy {
public:
  QStackedWidgetActionProxy() {}
  virtual ~QStackedWidgetActionProxy() {}

  ACTIONPROXY_NULL_RECURSE_DEF
};

} // namespace Tetradactyl
