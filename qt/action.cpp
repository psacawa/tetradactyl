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
#include <cstring>
#include <qassert.h>
#include <qlist.h>
#include <qobject.h>

#include <algorithm>
#include <iterator>
#include <map>
#include <qtpreprocessorsupport.h>

#include "action.h"
#include "actionmacros.h"
#include "hint.h"
#include "logging.h"

using std::map;

LOGGING_CATEGORY_COLOR("tetradactyl.action", Qt::magenta);

namespace Tetradactyl {

// ACTIONS

// To be replaced when actions need options
BaseAction *
BaseAction::createActionByHintMode(HintMode mode,
                                   WindowController *winController) {
  switch (mode) {
  case HintMode::Activatable:
    return new ActivateAction(winController);
  case HintMode::Editable:
    return new EditAction(winController);
  case HintMode::Focusable:
    return new FocusAction(winController);
  case HintMode::Yankable:
    return new YankAction(winController);
  default:
    logCritical << "BaseAction for HintMode" << mode << "not available in"
                << __PRETTY_FUNCTION__;
    Q_UNIMPLEMENTED();
  }
  Q_UNREACHABLE();
}

void BaseAction::accept(QWidget *widget) {
  QWidgetActionProxy *proxy = QWidgetActionProxy::getForMetaObject(
      winController->target()->metaObject());
  proxy->actGeneric(this, widget);
}

// ActivateAction

ActivateAction::ActivateAction(WindowController *controller)
    : BaseAction(controller) {
  mode = HintMode::Activatable;
}

void BaseAction::act() {
  QWidgetActionProxy *proxy = QWidgetActionProxy::getForMetaObject(
      winController->target()->metaObject());
  QList<HintData> hintData;
  proxy->hintGeneric(this, winController->target(), hintData);
  HintGenerator hintStringGenerator(Controller::settings.hintChars,
                                    hintData.length());
  for (auto datum : hintData) {
    string hintStr = *hintStringGenerator;
    logDebug << "Hinting " << datum.widget << " with " << hintStr;
    winController->mainOverlay()->addHint(
        QString::fromStdString(*hintStringGenerator), datum.widget,
        datum.point);
    hintStringGenerator++;
  }
  winController->mainOverlay()->setVisible(true);
  winController->mainOverlay()->resetSelection();
}

bool ActivateAction::done() { return true; }

// EditAction

EditAction::EditAction(WindowController *controller) : BaseAction(controller) {
  mode = HintMode::Editable;
}

bool EditAction::done() { return true; }

// FocusAction

FocusAction::FocusAction(WindowController *controller)
    : BaseAction(controller) {
  mode = HintMode::Focusable;
}

bool FocusAction::done() { return true; }

// YankAction

YankAction::YankAction(WindowController *controller) : BaseAction(controller) {
  mode = HintMode::Yankable;
}

bool YankAction::done() { return true; }

// // ContextMenuAction

// ContextMenuAction::ContextMenuAction(WindowController *controller)
//     : BaseAction(controller) {
//   mode = HintMode::Contextable;
// }

// QList<QWidget *> ContextMenuAction::getHintables(QWidget *root) { return; }

// WIDGET PROXIES

map<const QMetaObject *, QWidgetActionProxy *> QWidgetActionProxy::registry = {
    {&QLabel::staticMetaObject, new QLabelActionProxy},
    {&QAbstractButton::staticMetaObject, new QAbstractButtonActionProxy},
    {&QGroupBox::staticMetaObject, new QAbstractButtonActionProxy},
    {&QWidget::staticMetaObject, new QWidgetActionProxy},
    {&QTabBar::staticMetaObject, new QTabBarActionProxy},
    {&QStackedWidget::staticMetaObject, new QStackedWidgetActionProxy},
    {&QLineEdit::staticMetaObject, new QLineEditActionProxy}};

QWidgetActionProxy *
QWidgetActionProxy::getForMetaObject(const QMetaObject *mo) {
  const QMetaObject *iter = mo;
  while (iter != &QWidget::staticMetaObject) {
    auto search = registry.find(iter);
    if (search != registry.end()) {
      return search->second;
    }
    iter = iter->superClass();
  }
  // Send a warning if a base Qt widget had no ActionProxy. This is detected by
  // the first ancestor of the widget (in the sense of inheritance) having a
  // className starting with "Q".
  if (mo != &QWidget::staticMetaObject) {
    const QMetaObject *iter = mo;
    while (strncmp(iter->className(), "Q", 1) != 0)
      iter = iter->superClass();
    logWarning << "No ActionProxy class found for QMetaObject of"
               << mo->className() << "which inherits" << iter->className();
  }

  return registry.at(&QWidget::staticMetaObject);
}

// QWidgetActionProxy

bool QWidgetActionProxy::actGeneric(BaseAction *action, QWidget *widget) {
  switch (action->mode) {
  case Activatable:
    return this->activate(qobject_cast<ActivateAction *>(action), widget);
  case Editable:
    return this->edit(qobject_cast<EditAction *>(action), widget);
  case Focusable:
    return this->focus(qobject_cast<FocusAction *>(action), widget);
  case Yankable:
    return this->yank(qobject_cast<YankAction *>(action), widget);
  case Contextable:
    return this->contextMenu(qobject_cast<ContextMenuAction *>(action), widget);
  default:
    logCritical << "BaseAction for HintMode" << action->mode
                << "not available in" << __PRETTY_FUNCTION__;
    Q_UNIMPLEMENTED();
  }
  Q_UNREACHABLE();
}

void QWidgetActionProxy::hintGeneric(BaseAction *action, QWidget *widget,
                                     QList<HintData> &ret) {
  switch (action->mode) {
  case Activatable:
    return this->hintActivatable(qobject_cast<ActivateAction *>(action), widget,
                                 ret);
  case Editable:
    return this->hintEditable(qobject_cast<EditAction *>(action), widget, ret);
  case Focusable:
    return this->hintFocusable(qobject_cast<FocusAction *>(action), widget,
                               ret);
  case Yankable:
    return this->hintYankable(qobject_cast<YankAction *>(action), widget, ret);
  case Contextable:
    return this->hintContextMenuable(qobject_cast<ContextMenuAction *>(action),
                                     widget, ret);
  default:
    logCritical << "BaseAction for HintMode" << action->mode
                << "not available in" << __PRETTY_FUNCTION__;
    Q_UNIMPLEMENTED();
  }
  Q_UNREACHABLE();
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
      // not interested in Tetradactyl's widgets
      if (mo == &HintLabel::staticMetaObject ||
          mo == &Overlay::staticMetaObject)
        continue;

      QWidgetActionProxy *proxy = QWidgetActionProxy::getForMetaObject(mo);
      if (proxy->isActivatable(action, widget))
        ret.append(HintData{widget});

      proxy->hintActivatable(action, widget, ret);
    }
  }
}

// For now, this is very WET. We can eventually use PMFs to  kill these copied
// implementations.
void QWidgetActionProxy::hintEditable(EditAction *action, QWidget *widget,
                                      QList<HintData> &ret) {
  for (auto child : widget->children()) {
    QWidget *widget = qobject_cast<QWidget *>(child);
    if (widget && widget->isVisible() && widget->isEnabled()) {
      const QMetaObject *mo = widget->metaObject();
      // not interested in Tetradactyl's widgets
      if (mo == &HintLabel::staticMetaObject ||
          mo == &Overlay::staticMetaObject)
        continue;

      QWidgetActionProxy *proxy = QWidgetActionProxy::getForMetaObject(mo);
      if (proxy->isEditable(action, widget))
        ret.append(HintData{widget});

      proxy->hintEditable(action, widget, ret);
    }
  }
}

void QWidgetActionProxy::hintYankable(YankAction *action, QWidget *widget,
                                      QList<HintData> &ret) {
  for (auto child : widget->children()) {
    QWidget *widget = qobject_cast<QWidget *>(child);
    if (widget && widget->isVisible() && widget->isEnabled()) {
      const QMetaObject *mo = widget->metaObject();
      // not interested in Tetradactyl's widgets
      if (mo == &HintLabel::staticMetaObject ||
          mo == &Overlay::staticMetaObject)
        continue;

      QWidgetActionProxy *proxy = QWidgetActionProxy::getForMetaObject(mo);
      if (proxy->isYankable(action, widget))
        ret.append(HintData{widget});

      proxy->hintYankable(action, widget, ret);
    }
  }
}

void QWidgetActionProxy::hintFocusable(FocusAction *action, QWidget *widget,
                                       QList<HintData> &ret) {
  for (auto child : widget->children()) {
    QWidget *widget = qobject_cast<QWidget *>(child);
    if (widget && widget->isVisible() && widget->isEnabled()) {
      const QMetaObject *mo = widget->metaObject();
      // not interested in Tetradactyl's widgets
      if (mo == &HintLabel::staticMetaObject ||
          mo == &Overlay::staticMetaObject)
        continue;

      QWidgetActionProxy *proxy = QWidgetActionProxy::getForMetaObject(mo);
      if (proxy->isFocusable(action, widget))
        ret.append(HintData{widget});

      proxy->hintFocusable(action, widget, ret);
    }
  }
}

// QAbstractButtonActionProxy

bool QAbstractButtonActionProxy::activate(ActivateAction *action,
                                          QWidget *widget) {
  QOBJECT_CAST_ASSERT(QAbstractButton, widget);
  instance->setDown(true);
  instance->click();
  return true;
}

bool QAbstractButtonActionProxy::focus(FocusAction *action, QWidget *widget) {
  QOBJECT_CAST_ASSERT(QAbstractButton, widget);
  QWidgetActionProxy::focus(action, widget);
  instance->setDown(true);
  return true;
}

bool QAbstractButtonActionProxy::yank(YankAction *action, QWidget *widget) {
  QOBJECT_CAST_ASSERT(QAbstractButton, widget);
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setText(instance->text());
  return true;
}

// QGroupBoxActionProxy

bool QGroupBoxActionProxy::isActivatable(ActivateAction *action,
                                         QWidget *widget) {
  return qobject_cast<QGroupBox *>(widget)->isCheckable();
}

bool QGroupBoxActionProxy::activate(ActivateAction *action, QWidget *widget) {
  QOBJECT_CAST_ASSERT(QGroupBox, widget);
  instance->setChecked(!instance->isChecked());
  return true;
}

// QLabelActionProxy

bool QLabelActionProxy::yank(YankAction *action, QWidget *widget) {
  QOBJECT_CAST_ASSERT(QLabel, widget);
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setText(instance->text());
  return true;
}

// QTabBarActionProxy

void QTabBarActionProxy::hintActivatable(ActivateAction *action,
                                         QWidget *widget,
                                         QList<HintData> &ret) {
  QOBJECT_CAST_ASSERT(QTabBar, widget);
  auto tabHintLocations = probeTabLocations(instance);
  for (int idx = 0; idx != instance->count(); ++idx) {
    if (instance->isTabVisible(idx) && instance->isTabEnabled(idx)) {
      ret.push_back(
          HintData{.widget = instance, .point = tabHintLocations.at(idx)});
    }
  }
}

bool QTabBarActionProxy::activate(ActivateAction *action, QWidget *widget) {
  QOBJECT_CAST_ASSERT(QTabBar, widget);
  instance->setCurrentIndex(1);
  return true;
}

// Very silly code that dynamically probes tab positions until we set up the
// style spy to supply this information.
QList<QPoint> QTabBarActionProxy::probeTabLocations(QTabBar *bar) {
  QList<QPoint> ret;
  const int step = 5;
  int currentIdx = 0;
  for (int x = 0; x != bar->rect().width(); x += step) {

    if (bar->tabAt(QPoint(x, 0)) == currentIdx) {
      int i = 0;
      for (; i != step + 1; ++i) {
        if (bar->tabAt(QPoint(x - i, 0)) != currentIdx) {
          break;
        }
      }
      ret.push_back(QPoint(x - i + 1, 0));
      currentIdx++;
    }
  }
  return ret;
}

} // namespace Tetradactyl
