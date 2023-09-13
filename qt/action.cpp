// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractButton>
#include <QClipboard>
#include <QGroupBox>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QTabWidget>
#include <QTextEdit>
#include <QWidget>
#include <qlist.h>
#include <qobject.h>

#include <algorithm>
#include <iterator>
#include <map>

#include "action.h"

using std::map;

namespace Tetradactyl {

// ACTIONS

map<HintMode, AbstractUiAction *> actionRegistry = {{}};

AbstractUiAction *AbstractUiAction::createActionByHintMode(HintMode mode) {
  for (auto [mode_iter, action] : actionRegistry) {
    if (mode_iter == mode)
      return action;
  }
  Q_UNREACHABLE();
}

// ActivateAction

const QList<const QMetaObject *> ActivateAction::acceptableMetaObjects = {
    &QAbstractButton::staticMetaObject};

ActivateAction::ActivateAction(WindowController *controller)
    : AbstractUiAction(controller) {
  mode = HintMode::Activatable;
}

void ActivateAction::getHintables(QWidget *root, QList<HintData> &ret) {}

void ActivateAction::accept(QWidget *widget) {
  QAbstractButton *button = qobject_cast<QAbstractButton *>(widget);
  button->click();
}

bool ActivateAction::done() { return true; }

// EditAction

const QList<const QMetaObject *> EditAction::acceptableMetaObjects = {
    &QLineEdit::staticMetaObject, &QTextEdit::staticMetaObject};

EditAction::EditAction(WindowController *controller)
    : AbstractUiAction(controller) {
  mode = HintMode::Editable;
}

void EditAction::getHintables(QWidget *root, QList<HintData> &ret) {
  /*
   *   QList<HintData> descendants = root->findChildren<QWidget *>();
   *   auto filterLambda = [](QWidget *widget) {
   *     const QMetaObject *candidateMO = widget->metaObject();
   *     if (!widget->isVisible() || !widget->isEnabled())
   *       return false;
   *
   *     for (auto mo : acceptableMetaObjects) {
   *       if (candidateMO->inherits(mo)) {
   *         return true;
   *       }
   *     }
   *     return true;
   *   };
   *   copy_if(descendants.begin(), descendants.end(), std::back_inserter(ret),
   *           filterLambda);
   */
}
void EditAction::accept(QWidget *widget) { widget->setFocus(); }

// FocusAction

FocusAction::FocusAction(WindowController *controller)
    : AbstractUiAction(controller) {
  mode = HintMode::Focusable;
}

void FocusAction::getHintables(QWidget *root, QList<HintData> &list) { return; }

void FocusAction::accept(QWidget *widget) {}

// YankAction

YankAction::YankAction(WindowController *controller)
    : AbstractUiAction(controller) {
  mode = HintMode::Yankable;
}

void YankAction::getHintables(QWidget *root, QList<HintData> &list) { return; }

// ContextMenuAction

ContextMenuAction::ContextMenuAction(WindowController *controller)
    : AbstractUiAction(controller) {
  mode = HintMode::Contextable;
}

void ContextMenuAction::getHintables(QWidget *root, QList<HintData> &list) {
  return;
}

// WIDGET PROXIES

map<const QMetaObject *, QWidgetActionProxy *> QWidgetActionProxy::registry =
    {};

QWidgetActionProxy *
QWidgetActionProxy::getForMetaObject(const QMetaObject *mo) {
  while (mo != &QWidget::staticMetaObject) {
    auto search = registry.find(mo);
    if (search != registry.end()) {
      return search->second;
    }
    mo = mo->superClass();
  }
  return registry.at(&QWidget::staticMetaObject);
}

bool QWidgetActionProxy::isContextMenuable(ContextMenuAction *action,
                                           QWidget *widget) {
  // This leaves the uncommon possibilty that the client implemented a context
  // menu by implementing contextMenuEvent(QContextMenuEvent*) on the widget
  // subclass.
  return widget->contextMenuPolicy() == Qt::ActionsContextMenu ||
         widget->contextMenuPolicy() == Qt::CustomContextMenu;
}

void QWidgetActionProxy::hintActivatable(ActivateAction *action,
                                         QWidget *widget,
                                         QList<HintData> &ret) {
  for (auto child : widget->children()) {
    QWidget *widget = qobject_cast<QWidget *>(child);
    if (widget && widget->isVisible() && widget->isEnabled()) {
      const QMetaObject *mo = widget->metaObject();
      QWidgetActionProxy *proxy = QWidgetActionProxy::getForMetaObject(mo);
      if (proxy->isActivatable(action, widget)) {
        ret.append(HintData{widget});
      }
      proxy->hintActivatable(action, widget, ret);
    }
  }
}

// QGroupBoxActionProxy

bool QGroupBoxActionProxy::isActivatable(ActivateAction *action,
                                         QWidget *widget) {
  return qobject_cast<QGroupBox *>(widget)->isCheckable();
}

bool QGroupBoxActionProxy::activate(ActivateAction *action, QWidget *widget) {
  QGroupBox *groupBox = qobject_cast<QGroupBox *>(widget);
  groupBox->setChecked(groupBox->isChecked());
  return true;
}


bool QLabelActionProxy::yank(YankAction *action, QWidget *widget) {
  QLabel *instance = qobject_cast<QLabel *>(widget);
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setText(instance->text());
  return true;
}

} // namespace Tetradactyl
