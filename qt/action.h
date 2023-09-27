// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QList>
#include <QMenu>
#include <QMetaObject>
#include <QModelIndex>
#include <QStackedWidget>
#include <QTabBar>
#include <QTableView>
#include <QWidget>

#include <map>
#include <qabstractbutton.h>
#include <qobject.h>

// c++ forbids forward reference to enum HintMode
#include "controller.h"

#include "actionmacros.h"
#include "common.h"

using std::map;

namespace Tetradactyl {

// Actions

// Facilitates multi-stage actions, such as menu navigation. Also hold context
// that informs how the hinting procedure proceeds.

class BaseAction : public QObject {
  Q_OBJECT
public:
  Q_PROPERTY(HintMode mode MEMBER mode);
  Q_PROPERTY(WindowController *windowController MEMBER windowController);
  Q_PROPERTY(QWidget *currentRoot READ currentRoot);
  Q_PROPERTY(bool done READ isDone WRITE setDone);
  Q_PROPERTY(ControllerMode controllerModeAfterSuccess READ
                 controllerModeAfterSuccess);

  BaseAction(WindowController *controller) : windowController(controller) {
    p_currentRoot = windowController->target();
  }
  virtual ~BaseAction() {}
  bool isDone();
  ControllerMode controllerModeAfterSuccess();
  QWidget *currentRoot();

  static BaseAction *createActionByHintMode(HintMode, WindowController *);
public slots:
  virtual void accept(QWidgetActionProxy *proxy);
  void addNextStage(QWidget *root);
  void setDone(bool);
  void finish();

  virtual void act();

public:
  HintMode mode;
  WindowController *windowController;

signals:
  void stateChanged();
  void finished();

protected:
  bool done = false;
  ControllerMode p_controllerModeAfterSuccess = Normal;
  // Typically this corresponds to the the window widget of the
  // WindowController. That's the default. In the case of multi-step actions, it
  // may point to a QMenu Overlay
  QWidget *p_currentRoot;
};

inline bool BaseAction::isDone() { return done; }
inline void BaseAction::setDone(bool _done) { done = _done; }
inline void BaseAction::finish() { setDone(true); }
inline QWidget *BaseAction::currentRoot() { return p_currentRoot; }
inline ControllerMode BaseAction::controllerModeAfterSuccess() {
  return p_controllerModeAfterSuccess;
}

class ActivateAction : public BaseAction {
  Q_OBJECT
public:
  ActivateAction(WindowController *controller);
  virtual ~ActivateAction() {}
  // virtual bool isDone() override;
};

class EditAction : public BaseAction {
  Q_OBJECT
public:
  EditAction(WindowController *controller);
  virtual ~EditAction() {}
  // virtual bool isDone() override;
};

class FocusAction : public BaseAction {
  Q_OBJECT
public:
  FocusAction(WindowController *controller);
  virtual ~FocusAction() {}
  // virtual bool isDone() override;
};

class YankAction : public BaseAction {
  Q_OBJECT
public:
  YankAction(WindowController *controller);
  virtual ~YankAction() {}
  // virtual bool isDone() override;
};

class ContextMenuAction : public BaseAction {
  Q_OBJECT
public:
  ContextMenuAction(WindowController *controller);
  virtual ~ContextMenuAction() {}
  // bool isDone();
};

class MenuBarAction : public BaseAction {
  Q_OBJECT
public:
  Q_PROPERTY(QList<QMenu *> menusToClose MEMBER menusToClose);
  MenuBarAction(WindowController *controller);
  virtual ~MenuBarAction() {}

  // For nested menus, keep a record of those menus that have to be hidden once
  // the action is finished. Is there a native API for this?
  QList<QMenu *> menusToClose;
};

// Widget Proxies

// A speculative idea: The action-specific code is accompanoed by an
// inheritance hierarchy of "widget proxy" classes that mirrors the QWidget
// inheritance hierarchy. These tell whether/how to implement the routines
// in a manner specific to the widget, while still permitting code reuse via
// the inheritance. Context is passed to the actor methods via the actions,
// which have a context attached to them.

// When visiting a widget, you may or may not want to hint it. Separately,
// you may or may not want to recurse under it to search for more hints.
// Notwithstanding some edge cases, typically a hinted recursed won't be
// recursed under. The methods is*able() determine whether the widget itself
// should generate a hint. The caller does the adding. Conversely, the
// methods hint*able For now, entering is*able and hint*able methods means
// that the widget is enabled and visible: hint*able is responsible for
// calling visible(QWidget*) for the children it wishes to recurse to.

// Therefore, according to current *doctrine*, e.g. a QTabBar would handle
// hintable tabs and recurse to the underlying QStackedWidget in
// hintActivatable. Itself, it is not activatable. Likewise, QMenu handles
// it's options in hintActivatable. NB this holds whether or not the
// hintable "subwidgets" are real widgets in the true sense, or rather are
// pseudo-widgets implemented by QStyle::drawControl etc. (short-sighthed?)
// This doctrine lasts until the next surprise.

// In actionmacros.h we define preprocessor macros to cut  down on
// boilerplate code for each QWidget subclass.

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
  virtual bool isMenuable(MenuBarAction *action, QWidget *widget) {
    return false;
  }

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
  virtual void hintMenuable(MenuBarAction *action, QWidget *widget,
                            QList<QWidgetActionProxy *> &proxies);
};

struct WidgetHintingData {
  const QMetaObject *actionProxyMO;
  QWidgetActionProxyStatic *staticMethods;
};

extern map<const QMetaObject *, WidgetHintingData> QWidgetMetadataRegistry;

const WidgetHintingData getMetadataForMetaObject(const QMetaObject *mo);

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
  virtual bool menu(MenuBarAction *action) { return false; }
  virtual bool contextMenu(ContextMenuAction *action);

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

// QComboBoxActionProxy

class QComboBoxActionProxyStatic : public QWidgetActionProxyStatic {
  bool isActivatable(ActivateAction *action, QWidget *widget) override;
  bool isEditable(EditAction *action, QWidget *widget) override;
  ACTIONPROXY_TRUE_SELF_FOCUSABLE_DEF
  ACTIONPROXY_TRUE_SELF_YANKABLE_DEF
};

class QComboBoxActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QComboBoxActionProxy(QWidget *w) : QWidgetActionProxy(w) {}

  bool activate(ActivateAction *action) override;
  // bool focus(FocusAction *action) override;
  bool yank(YankAction *action) override;
};

// QGroupBoxActionProxy

class QGroupBoxActionProxyStatic : public QWidgetActionProxyStatic {

  virtual bool isActivatable(ActivateAction *action, QWidget *widget) override;
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
  bool isEditable(EditAction *action, QWidget *widget) override;
  ACTIONPROXY_TRUE_SELF_FOCUSABLE_DEF
};

class QLineEditActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QLineEditActionProxy(QWidget *w) : QWidgetActionProxy(w) {}

  ACTIONPROXY_DEFAULT_ACTION_EDITABLE_DEF
};

// QMenuBarActionProxy

class QMenuBarActionProxyStatic : public QWidgetActionProxyStatic {
public:
  void hintMenuable(MenuBarAction *action, QWidget *widget,
                    QList<QWidgetActionProxy *> &proxies) override;
};

class QMenuBarActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QMenuBarActionProxy(QWidget *w, QPoint position,
                                  QAction *_menuAction)
      : QWidgetActionProxy(w, position), menuAction(_menuAction) {}
  virtual ~QMenuBarActionProxy() {}

  bool menu(MenuBarAction *action) override;

private:
  QAction *menuAction;
};

// QMenuActionProxy

class QMenuActionProxyStatic : public QWidgetActionProxyStatic {
public:
  void hintMenuable(MenuBarAction *action, QWidget *widget,
                    QList<QWidgetActionProxy *> &proxies) override;
};

class QMenuActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QMenuActionProxy(QWidget *w, QPoint position,
                               QAction *_menuAction)
      : QWidgetActionProxy(w, position), menuAction(_menuAction) {}
  virtual ~QMenuActionProxy() {}

  bool menu(MenuBarAction *action) override;

private:
  QAction *menuAction;
};

// QTabBarActionProxy

class QTabBarActionProxyStatic : public QWidgetActionProxyStatic {
  virtual void hintActivatable(ActivateAction *action, QWidget *widget,
                               QList<QWidgetActionProxy *> &proxies) override;
  void hintYankable(YankAction *action, QWidget *widget,
                    QList<QWidgetActionProxy *> &proxies) override;

  static QList<QPoint> probeTabLocations(QTabBar *bar);
  static void hintHelper(BaseAction *action, QWidget *widget,
                         QList<QWidgetActionProxy *> &proxies);
};

class QTabBarActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QTabBarActionProxy(int idx, QPoint positionInWidget, QWidget *w)
      : QWidgetActionProxy(w, positionInWidget), tabIndex(idx) {}
  virtual ~QTabBarActionProxy() {}

  bool activate(ActivateAction *action) override;
  bool yank(YankAction *action) override;

private:
  int tabIndex;
};

// QStackedWidgetActionProxy

class QStackedWidgetActionProxyStatic : public QWidgetActionProxyStatic {
  void hintActivatable(ActivateAction *action, QWidget *widget,
                       QList<QWidgetActionProxy *> &proxies) override;
  void hintEditable(EditAction *action, QWidget *widget,
                    QList<QWidgetActionProxy *> &proxies) override;
  void hintFocusable(FocusAction *action, QWidget *widget,
                     QList<QWidgetActionProxy *> &proxies) override;
  void hintYankable(YankAction *action, QWidget *widget,
                    QList<QWidgetActionProxy *> &proxies) override;
  void hintContextMenuable(ContextMenuAction *action, QWidget *widget,
                           QList<QWidgetActionProxy *> &proxies) override;
};

class QStackedWidgetActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QStackedWidgetActionProxy(QWidget *w) : QWidgetActionProxy(w) {}
  virtual ~QStackedWidgetActionProxy() {}
};

// QTextEditActionProxy

class QTextEditActionProxyStatic : public QWidgetActionProxyStatic {
  ACTIONPROXY_TRUE_SELF_FOCUSABLE_DEF
  bool isEditable(EditAction *action, QWidget *widget) override;
};

class QTextEditActionProxy : public QWidgetActionProxy {
  Q_OBJECT
public:
  Q_INVOKABLE QTextEditActionProxy(QWidget *w) : QWidgetActionProxy(w) {}

  ACTIONPROXY_DEFAULT_ACTION_EDITABLE_DEF
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
