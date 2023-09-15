// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QList>
#include <QMetaObject>
#include <QModelIndex>
#include <QStackedWidget>
#include <QTabBar>
#include <QTableView>
#include <QWidget>

#include <map>
#include <qabstractbutton.h>
#include <qassert.h>
#include <qobject.h>

// c++ forbids  forward reference to enum HintMode
#include "actionmacros.h"
#include "common.h"
#include "controller.h"

using std::map;

namespace Tetradactyl {

// Actions

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
  virtual void accept(QWidgetActionProxy *proxy);
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
  bool done();
};

// Widget Proxies

// A speculative idea: The action-specific code is accompanoed by an inheritance
// hierarchy of "widget proxy" classes that mirrors the QWidget inheritance
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

class QWidgetActionProxyStatic {
public:
  // can the widget itself be hinted/actioned?

  virtual bool isHintableGeneric(BaseAction *action, QWidget *widget);
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
                           QList<QWidgetActionProxy *> &proxies);
  virtual void hintActivatable(ActivateAction *action, QWidget *widget,
                               QList<QWidgetActionProxy *> &proxies);
  virtual void hintYankable(YankAction *action, QWidget *widget,
                            QList<QWidgetActionProxy *> &proxies);
  virtual void hintEditable(EditAction *action, QWidget *widget,
                            QList<QWidgetActionProxy *> &proxies);
  virtual void hintFocusable(FocusAction *action, QWidget *widget,
                             QList<QWidgetActionProxy *> &proxies);
  virtual void hintContextMenuable(ContextMenuAction *action, QWidget *widget,
                                   QList<QWidgetActionProxy *> &proxies);
};

struct WidgetHintingData {
  const QMetaObject *actionProxyMO;
  QWidgetActionProxyStatic *staticMethods;
};

extern map<const QMetaObject *, WidgetHintingData> QWidgetMetadataRegistry;

class QWidgetActionProxy : public QObject {
  Q_OBJECT
public:
  Q_INVOKABLE QWidgetActionProxy(QWidget *w,
                                 QPoint _positionInWidget = QPoint(0, 0))
      : widget(w), positionInWidget(_positionInWidget) {}
  virtual ~QWidgetActionProxy() {}

  inline static bool visible(QWidget *w);
  // how to action widget type?
  bool actGeneric(BaseAction *);
  virtual bool activate(ActivateAction *action) { return false; }
  virtual bool yank(YankAction *action) { return false; }
  virtual bool edit(EditAction *action) { return false; }
  virtual bool focus(FocusAction *action) {
    widget->setFocus();
    return true;
  }
  virtual bool contextMenu(ContextMenuAction *action) { return false; }

  static const WidgetHintingData
  getMetadataForMetaObject(const QMetaObject *mo);
  static QWidgetActionProxy *createForMetaObject(const QMetaObject *mo,
                                                 QWidget *w);

  QWidget *widget;
  QPoint positionInWidget;
};

inline bool QWidgetActionProxy::visible(QWidget *w) {
  return w->isVisible() && w->isEnabled();
}

// QAbstractButtonActionProxy

class QAbstractButtonActionProxyStatic : public QWidgetActionProxyStatic {
  ACTIONPROXY_TRUE_SELF_ACTIVATABLE_DEF
  ACTIONPROXY_TRUE_SELF_YANKABLE_DEF
  ACTIONPROXY_TRUE_SELF_FOCUSABLE_DEF

  ACTIONPROXY_NULL_RECURSE_DEF
};

class QAbstractButtonActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QAbstractButtonActionProxy(QWidget *w) : QWidgetActionProxy(w) {}
  virtual ~QAbstractButtonActionProxy() {}

  virtual bool activate(ActivateAction *action);
  virtual bool focus(FocusAction *action);
  virtual bool yank(YankAction *action);
};

// QGroupBoxActionProxy

class QGroupBoxActionProxyStatic : public QWidgetActionProxyStatic {

  virtual bool isActivatable(ActivateAction *action, QWidget *widget);
};

class QGroupBoxActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QGroupBoxActionProxy(QWidget *w) : QWidgetActionProxy(w) {}
  virtual bool activate(ActivateAction *action) override;
};

// QLabelActionProxy
class QLabelActionProxyStatic : public QWidgetActionProxyStatic {
  ACTIONPROXY_TRUE_SELF_YANKABLE_DEF
  ACTIONPROXY_NULL_RECURSE_DEF
};

class QLabelActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QLabelActionProxy(QWidget *w) : QWidgetActionProxy(w) {}
  virtual bool yank(YankAction *action) override;
};

// QLineEditActionProxy

class QLineEditActionProxyStatic : public QWidgetActionProxyStatic {
  ACTIONPROXY_TRUE_SELF_EDITABLE_DEF
  ACTIONPROXY_TRUE_SELF_FOCUSABLE_DEF
};

class QLineEditActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QLineEditActionProxy(QWidget *w) : QWidgetActionProxy(w) {}

  ACTIONPROXY_DEFAULT_ACTION_EDITABLE_DEF
};

// QTabBarActionProxy

class QTabBarActionProxyStatic : public QWidgetActionProxyStatic {
  virtual void hintActivatable(ActivateAction *action, QWidget *widget,
                               QList<QWidgetActionProxy *> &proxies) override;

  static QList<QPoint> probeTabLocations(QTabBar *bar);
};

class QTabBarActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QTabBarActionProxy(int idx, QPoint positionInWidget, QWidget *w)
      : QWidgetActionProxy(w, positionInWidget), tabIndex(idx) {}
  virtual ~QTabBarActionProxy() {}

  bool activate(ActivateAction *action) override;

private:
  int tabIndex;
};

// QStackedWidgetActionProxy

class QStackedWidgetActionProxyStatic : public QWidgetActionProxyStatic {
  ACTIONPROXY_NULL_RECURSE_DEF
};

class QStackedWidgetActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QStackedWidgetActionProxy(QWidget *w) : QWidgetActionProxy(w) {}
  virtual ~QStackedWidgetActionProxy() {}
};

// MODEL VIEW ACTION PROXIES

// QAbstractItemViewActionProxy

// What does activation mean for these views?

class QAbstractItemViewActionProxyStatic : public QWidgetActionProxyStatic {
public:
};

class QAbstractItemViewActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QAbstractItemViewActionProxy(QModelIndex idx,
                                           QPoint _positionInWidget, QWidget *w)
      : QWidgetActionProxy(w, _positionInWidget), modelIndex(idx) {}

  virtual bool edit(EditAction *action) override;
  virtual bool focus(FocusAction *action) override;

private:
  QModelIndex modelIndex;
  QPoint positionInWidget;
};

// QListViewActionProxy

class QListViewActionProxyStatic : public QAbstractItemViewActionProxyStatic {
public:
  virtual void hintEditable(EditAction *action, QWidget *widget,
                            QList<QWidgetActionProxy *> &proxies) override;
  virtual void hintFocusable(FocusAction *action, QWidget *widget,
                             QList<QWidgetActionProxy *> &proxies) override;
};
class QListViewActionProxy : public QAbstractItemViewActionProxy {
  Q_OBJECT
public:
  using QAbstractItemViewActionProxy::QAbstractItemViewActionProxy;
  virtual ~QListViewActionProxy() {}
};

// QTableViewActionProxy

class QTableViewActionProxyStatic : public QAbstractItemViewActionProxyStatic {
public:
  virtual void hintEditable(EditAction *action, QWidget *widget,
                            QList<QWidgetActionProxy *> &proxies) override;
  virtual void hintFocusable(FocusAction *action, QWidget *widget,
                             QList<QWidgetActionProxy *> &proxies) override;
};
class QTableViewActionProxy : public QAbstractItemViewActionProxy {
  Q_OBJECT
public:
  using QAbstractItemViewActionProxy::QAbstractItemViewActionProxy;
  virtual ~QTableViewActionProxy() {}
};

} // namespace Tetradactyl
