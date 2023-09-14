// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

// Casts widget to subclass klass. Set to  variable instance
#define QOBJECT_CAST_ASSERT(klass, widget)                                     \
  klass *instance = qobject_cast<klass *>(this->widget);                       \
  Q_ASSERT(instance != nullptr);

// Macros to declare/define inline widget hintable probing ActionProxy methods.

// FALSE: I'm not hintable

#define ACTIONPROXY_FALSE_SELF_ACTIVATABLE_DEF                                 \
  virtual bool isActivatable(ActivateAction *action, QWidget *widget)          \
      override {                                                               \
    return false;                                                              \
  }
#define ACTIONPROXY_FALSE_SELF_YANKABLE_DEF                                    \
  virtual bool isYankable(YankAction *action, QWidget *widget) override {      \
    return false;                                                              \
  }
#define ACTIONPROXY_FALSE_SELF_EDITABLE_DEF                                    \
  virtual bool isEditable(EditAction *action, QWidget *widget) override {      \
    return false;                                                              \
  }
#define ACTIONPROXY_FALSE_SELF_FOCUSABLE_DEF                                   \
  virtual bool isFocusable(FocusAction *action, QWidget *widget) override {    \
    return false;                                                              \
  }
#define ACTIONPROXY_FALSE_SELF_CONTEXT_MENUABLE_DEF                            \
  virtual bool isContextMenuable(ContextMenuAction *action, QWidget *widget)   \
      override {                                                               \
    return false;                                                              \
  }

// TRUE: I am hintable

#define ACTIONPROXY_TRUE_SELF_ACTIVATABLE_DEF                                  \
  virtual bool isActivatable(ActivateAction *action, QWidget *widget)          \
      override {                                                               \
    return true;                                                               \
  }
#define ACTIONPROXY_TRUE_SELF_YANKABLE_DEF                                     \
  virtual bool isYankable(YankAction *action, QWidget *widget) override {      \
    return true;                                                               \
  }
#define ACTIONPROXY_TRUE_SELF_EDITABLE_DEF                                     \
  virtual bool isEditable(EditAction *action, QWidget *widget) override {      \
    return true;                                                               \
  }
#define ACTIONPROXY_TRUE_SELF_FOCUSABLE_DEF                                    \
  virtual bool isFocusable(FocusAction *action, QWidget *widget) override {    \
    return true;                                                               \
  }
#define ACTIONPROXY_TRUE_SELF_CONTEXT_MENUABLE_DEF                             \
  virtual bool isContextMenuable(ContextMenuAction *action, QWidget *widget)   \
      override {                                                               \
    return true;                                                               \
  }

#define ACTIONPROXY_FALSE_SELF_DEF                                             \
  ACTIONPROXY_FALSE_SELF_ACTIVATABLE_DEF                                       \
  ACTIONPROXY_FALSE_SELF_YANKABLE_DEF                                          \
  ACTIONPROXY_FALSE_SELF_EDITABLE_DEF                                          \
  ACTIONPROXY_FALSE_SELF_FOCUSABLE_DEF                                         \
  ACTIONPROXY_FALSE_SELF_CONTEXT_MENUABLE_DEF

// Macros to declare/define inline widget recursion ActionProxy methods.

#define ACTIONPROXY_NULL_RECURSE_ACTIVATABLE_DEF                               \
  virtual void hintActivatable(ActivateAction *action, QWidget *widget,        \
                               QList<QWidgetActionProxy *> &ret) override {    \
    return;                                                                    \
  }
#define ACTIONPROXY_NULL_RECURSE_YANKABLE_DEF                                  \
  virtual void hintYankable(YankAction *action, QWidget *widget,               \
                            QList<QWidgetActionProxy *> &ret) override {       \
    return;                                                                    \
  }
#define ACTIONPROXY_NULL_RECURSE_EDITABLE_DEF                                  \
  virtual void hintEditable(EditAction *action, QWidget *widget,               \
                            QList<QWidgetActionProxy *> &ret) override {       \
    return;                                                                    \
  }
#define ACTIONPROXY_NULL_RECURSE_FOCUSABLE_DEF                                 \
  virtual void hintFocusable(FocusAction *action, QWidget *widget,             \
                             QList<QWidgetActionProxy *> &ret) override {      \
    return;                                                                    \
  }
#define ACTIONPROXY_NULL_RECURSE_CONTEXT_MENUABLE_DEF                          \
  virtual void hintContextMenuable(ContextMenuAction *action, QWidget *widget, \
                                   QList<QWidgetActionProxy *> &ret)           \
      override {                                                               \
    return;                                                                    \
  }

#define ACTIONPROXY_NULL_RECURSE_DEF                                           \
  ACTIONPROXY_NULL_RECURSE_ACTIVATABLE_DEF                                     \
  ACTIONPROXY_NULL_RECURSE_YANKABLE_DEF                                        \
  ACTIONPROXY_NULL_RECURSE_EDITABLE_DEF                                        \
  ACTIONPROXY_NULL_RECURSE_FOCUSABLE_DEF                                       \
  ACTIONPROXY_NULL_RECURSE_CONTEXT_MENUABLE_DEF

// Macros to declare/define inline action implementation ActionProxy methods.

#define ACTIONPROXY_NULL_ACTION_ACTIVATABLE_DEF                                \
  virtual bool activate(ActivateAction *action) override { return false; }
#define ACTIONPROXY_NULL_ACTION_YANKABLE_DEF                                   \
  virtual bool yank(YankAction *action) override { return false; }
#define ACTIONPROXY_NULL_ACTION_EDITABLE_DEF                                   \
  virtual bool edit(EditAction *action) override { return false; }
#define ACTIONPROXY_NULL_ACTION_FOCUSABLE_DEF                                  \
  virtual bool focus(FocusAction *action) override { return false; }
#define ACTIONPROXY_NULL_ACTION_CONTEXT_MENUABLE_DEF                           \
  virtual bool contextMenu(ContextMenuAction *action) override { return false; }

// Stub implementations which are *not* the implementations on
// QWidgetActionProxy. Mostly not even  useful.
#define ACTIONPROXY_DEFAULT_ACTION_ACTIVATABLE_DEF(klass)                      \
  virtual bool activate(ActivateAction *action) override {                     \
    (klass) *instance = qobject_cast<(klass) *>(widget);                       \
    instance->click();                                                         \
    return true;                                                               \
  }
#define ACTIONPROXY_DEFAULT_ACTION_YANKABLE_DEF(klass)                         \
  virtual bool yank(YankAction *action) override {                             \
    (klass) *instance = qobject_cast<(klass) *>(widget);                       \
    QClipboard *clipboard = QGuiApplication::clipboard();                      \
    clipboard->setText(instance->text());                                      \
    return true;                                                               \
  }

// May be used eventually to launch external editor via IME
#define ACTIONPROXY_DEFAULT_ACTION_EDITABLE_DEF                                \
  virtual bool edit(EditAction *action) override {                             \
    widget->setFocus();                                                        \
    return true;                                                               \
  }
#define ACTIONPROXY_DEFAULT_ACTION_FOCUSABLE_DEF                               \
  virtual bool focus(FocusAction *action) override {                           \
    widget->setFocus();                                                        \
    return true;                                                               \
  }
// TODO 13/09/20 psacawa: how to trigger context menu
#define ACTIONPROXY_DEFAULT_ACTION_CONTEXT_MENUABLE_DEF(klass)                 \
  virtual bool contextMenu(ContextMenuAction *action) override { return false; }

#define ACTIONPROXY_NULL_ACTION_DEF                                            \
  ACTIONPROXY_NULL_ACTION_ACTIVATABLE_DEF                                      \
  ACTIONPROXY_NULL_ACTION_YANKABLE_DEF                                         \
  ACTIONPROXY_NULL_ACTION_EDITABLE_DEF                                         \
  ACTIONPROXY_NULL_ACTION_FOCUSABLE_DEF                                        \
  ACTIONPROXY_NULL_ACTION_CONTEXT_MENUABLE_DEF
