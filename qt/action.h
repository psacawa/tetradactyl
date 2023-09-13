// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QList>
#include <QMetaObject>
#include <QWidget>

#include <map>

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
class AbstractUiAction {
public:
  HintMode mode;
  WindowController *winController;
  virtual void getHintables(QWidget *root, QList<HintData> &list) = 0;
  AbstractUiAction(WindowController *controller) : winController(controller) {}
  virtual ~AbstractUiAction() {}
  virtual void accept(QWidget *widget) = 0;
  virtual bool done() = 0;

  static AbstractUiAction *createActionByHintMode(HintMode);
};

class ActivateAction : public AbstractUiAction {
public:
  static const QList<const QMetaObject *> acceptableMetaObjects;
  ActivateAction(WindowController *controller);
  void getHintables(QWidget *root, QList<HintData> &list) override;
  void accept(QWidget *widget) override;
  virtual bool done() override;
  virtual ~ActivateAction() {}
};

class EditAction : public AbstractUiAction {
public:
  static const QList<const QMetaObject *> acceptableMetaObjects;
  EditAction(WindowController *controller);
  void getHintables(QWidget *root, QList<HintData> &list) override;
  void accept(QWidget *widget) override;
  virtual ~EditAction() {}
};

class FocusAction : public AbstractUiAction {
public:
  static const QList<const QMetaObject *> acceptableMetaObjects;
  FocusAction(WindowController *controller);
  void getHintables(QWidget *root, QList<HintData> &list) override;
  void accept(QWidget *widget) override;
  virtual ~FocusAction() {}
};

class YankAction : public AbstractUiAction {
public:
  static const QList<const QMetaObject *> acceptableMetaObjects;
  YankAction(WindowController *controller);
  void getHintables(QWidget *root, QList<HintData> &list) override;
  virtual ~YankAction() {}
};

class ContextMenuAction : public AbstractUiAction {
public:
  static const QList<const QMetaObject *> acceptableMetaObjects;
  ContextMenuAction(WindowController *controller);
  void getHintables(QWidget *root, QList<HintData> &list) override;
  virtual ~ContextMenuAction() {}
};

extern map<HintMode, AbstractUiAction *> actionRegistry;

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

// Therefore, according to current *doctrine*, e.g. a TabWidget would handle
// hintable tabs and  recurse to the underlying QStackedWdiget in
// hintActivatable. Itself, it is not activatable. Likewise, QMenu handles it's
// options in hintActivatable. This doctrine lasts until the next surprise.

// In actionmacros.h we define preprocessor macros to cut  down on boilerplate
// code for each QWidget subclass.

//
// QWidgetActionProxy
//

class QWidgetActionProxy {
public:
  QWidgetActionProxy() {}
  virtual ~QWidgetActionProxy() {}

  inline static bool visible(QWidget *w) {
    return w->isVisible() && w->isEnabled();
  }

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
  virtual void hintActivatable(ActivateAction *action, QWidget *widget,
                               QList<HintData> &ret);
  virtual void hintYankable(YankAction *action, QWidget *widget,
                            QList<HintData> &ret) {
    return;
  }
  virtual void hintEditable(EditAction *action, QWidget *widget,
                            QList<HintData> &ret) {
    return;
  }
  virtual void hintFocusable(FocusAction *action, QWidget *widget,
                             QList<HintData> &ret) {
    return;
  }
  virtual void hintContextMenuable(ContextMenuAction *action, QWidget *widget,
                                   QList<HintData> &ret) {
    return;
  }

  // how to action widget type?
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

//
// QAbstractButtonActionProxy
//

class QAbstractButtonActionProxy : QWidgetActionProxy {
public:
  QAbstractButtonActionProxy() {}
  virtual ~QAbstractButtonActionProxy() {}

  ACTIONPROXY_TRUE_SELF_ACTIVATABLE_DEF
  ACTIONPROXY_TRUE_SELF_YANKABLE_DEF
  ACTIONPROXY_TRUE_SELF_FOCUSABLE_DEF

  ACTIONPROXY_NULL_RECURSE_DEF

  virtual bool activate(ActivateAction *action, QWidget *widget) override {
    return false;
  }
  virtual bool yank(YankAction *action, QWidget *widget) override {
    return false;
  }
};

class QLabelActionProxy : QWidgetActionProxy {
public:
  ACTIONPROXY_TRUE_SELF_YANKABLE_DEF
  ACTIONPROXY_NULL_RECURSE_DEF
  virtual bool yank(YankAction *action, QWidget *widget) override;
};

class QGroupBoxActionProxy : QWidgetActionProxy {
public:
  virtual bool isActivatable(ActivateAction *action, QWidget *widget) override;
  virtual bool activate(ActivateAction *action, QWidget *widget) override;
};

} // namespace Tetradactyl
