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
#include <qminmax.h>
#include <qobject.h>

#include <algorithm>
#include <iterator>
#include <map>
#include <qtpreprocessorsupport.h>

#include "action.h"
#include "actionmacros.h"
#include "common.h"
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
  case Activatable:
    return new ActivateAction(winController);
  case Editable:
    return new EditAction(winController);
  case Focusable:
    return new FocusAction(winController);
  case Yankable:
    return new YankAction(winController);
  case Contextable:
    return new ContextMenuAction(winController);
  default:
    logCritical << "BaseAction for HintMode" << mode << "not available in"
                << __PRETTY_FUNCTION__;
    Q_UNIMPLEMENTED();
  }
  Q_UNREACHABLE();
}

void BaseAction::accept(QWidgetActionProxy *proxy) {
  // TODO 14/09/20 psacawa: remove widget from signature
  proxy->actGeneric(this);
}

// ActivateAction

ActivateAction::ActivateAction(WindowController *controller)
    : BaseAction(controller) {
  mode = HintMode::Activatable;
}

void BaseAction::act() {
  // QWidgetActionProxy *proxy = QWidgetActionProxy::createForMetaObject(
  //     winController->target()->metaObject());
  QList<QWidgetActionProxy *> hintData;
  // get the hints
  const QMetaObject *targetMO = winController->target()->metaObject();
  auto metadata = QWidgetActionProxy::getMetadataForMetaObject(targetMO);
  metadata.staticMethods->hintGeneric(this, winController->target(), hintData);
  HintGenerator hintStringGenerator(Controller::settings.hintChars,
                                    hintData.length());
  for (QWidgetActionProxy *actionProxy : hintData) {
    string hintStr = *hintStringGenerator;
    logDebug << "Hinting " << actionProxy->widget << " with " << hintStr;
    winController->mainOverlay()->addHint(
        QString::fromStdString(*hintStringGenerator), actionProxy);
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

// ContextMenuAction

ContextMenuAction::ContextMenuAction(WindowController *controller)
    : BaseAction(controller) {
  mode = HintMode::Contextable;
}

bool ContextMenuAction::done() { return true; }

// QList<QWidget *> ContextMenuAction::getHintables(QWidget *root) { return; }

// WIDGET PROXIES

// A horrific static that enables us to use dynamic dispatch of static methods
// dependent on a widget's QMetaObject. We get the static methods and
// ActionProxies via this map.  Find a better way!
map<const QMetaObject *, WidgetHintingData> QWidgetMetadataRegistry = {
    {&QLabel::staticMetaObject,
     {&QLabelActionProxy::staticMetaObject, new QLabelActionProxyStatic}},
    {&QAbstractButton::staticMetaObject,
     {&QAbstractButtonActionProxy::staticMetaObject,
      new QAbstractButtonActionProxyStatic}},
    {&QGroupBox::staticMetaObject,
     {&QGroupBoxActionProxy::staticMetaObject, new QGroupBoxActionProxyStatic}},
    {&QWidget::staticMetaObject,
     {&QWidgetActionProxy::staticMetaObject, new QWidgetActionProxyStatic}},
    {&QTabBar::staticMetaObject,
     {&QTabBarActionProxy::staticMetaObject, new QTabBarActionProxyStatic}},
    {&QStackedWidget::staticMetaObject,
     {&QStackedWidgetActionProxy::staticMetaObject,
      new QStackedWidgetActionProxyStatic}},
    {&QLineEdit::staticMetaObject,
     {&QLineEditActionProxy::staticMetaObject, new QLineEditActionProxyStatic}},
    {&QTableView::staticMetaObject,
     {&QTableViewActionProxy::staticMetaObject,
      new QTableViewActionProxyStatic}}};

const WidgetHintingData
QWidgetActionProxy::getMetadataForMetaObject(const QMetaObject *widgetMO) {

  WidgetHintingData metadata =
      QWidgetMetadataRegistry.at(&QWidget::staticMetaObject);
  const QMetaObject *iter = widgetMO;
  while (iter != &QWidget::staticMetaObject) {
    auto search = QWidgetMetadataRegistry.find(iter);
    if (search != QWidgetMetadataRegistry.end())
      metadata = search->second;

    iter = iter->superClass();
  }
  // Send a warning if a base Qt widget had no ActionProxy. This is detected by
  // the first ancestor of the widget (in the sense of inheritance) having a
  // className starting with "Q".
  if (widgetMO != &QWidget::staticMetaObject &&
      metadata.actionProxyMO == &QWidgetActionProxy::staticMetaObject) {
    const QMetaObject *iter = widgetMO;
    while (strncmp(iter->className(), "Q", 1) != 0)
      iter = iter->superClass();
    logWarning << "No ActionProxy class found for QMetaObject of"
               << widgetMO->className() << "which inherits"
               << iter->className();
  }

  return metadata;
}

QWidgetActionProxy *
QWidgetActionProxy::createForMetaObject(const QMetaObject *widgetMO,
                                        QWidget *w) {
  WidgetHintingData metadata = getMetadataForMetaObject(widgetMO);
  QObject *obj = metadata.actionProxyMO->newInstance(w);
  Q_ASSERT(obj != nullptr);
  return reinterpret_cast<QWidgetActionProxy *>(obj);
}

// QWidgetActionProxy

bool QWidgetActionProxyStatic::isHintableGeneric(BaseAction *action,
                                                 QWidget *widget) {
  switch (action->mode) {
  case Activatable:
    return isActivatable(qobject_cast<ActivateAction *>(action), widget);
  case Editable:
    return isEditable(qobject_cast<EditAction *>(action), widget);
  case Focusable:
    return isFocusable(qobject_cast<FocusAction *>(action), widget);
  case Yankable:
    return isYankable(qobject_cast<YankAction *>(action), widget);
  case Contextable:
    return isContextMenuable(qobject_cast<ContextMenuAction *>(action), widget);
  default:
    logCritical << "BaseAction for HintMode" << action->mode
                << "not available in" << __PRETTY_FUNCTION__;
    Q_UNIMPLEMENTED();
  }
  Q_UNREACHABLE();
}

bool QWidgetActionProxy::actGeneric(BaseAction *action) {
  switch (action->mode) {
  case Activatable:
    return this->activate(qobject_cast<ActivateAction *>(action));
  case Editable:
    return this->edit(qobject_cast<EditAction *>(action));
  case Focusable:
    return this->focus(qobject_cast<FocusAction *>(action));
  case Yankable:
    return this->yank(qobject_cast<YankAction *>(action));
  case Contextable:
    return this->contextMenu(qobject_cast<ContextMenuAction *>(action));
  default:
    logCritical << "BaseAction for HintMode" << action->mode
                << "not available in" << __PRETTY_FUNCTION__;
    Q_UNIMPLEMENTED();
  }
  Q_UNREACHABLE();
}

void QWidgetActionProxyStatic::hintGeneric(
    BaseAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
  switch (action->mode) {
  case Activatable:
    return hintActivatable(qobject_cast<ActivateAction *>(action), widget,
                           proxies);
  case Editable:
    return hintEditable(qobject_cast<EditAction *>(action), widget, proxies);
  case Focusable:
    return hintFocusable(qobject_cast<FocusAction *>(action), widget, proxies);
  case Yankable:
    return hintYankable(qobject_cast<YankAction *>(action), widget, proxies);
  case Contextable:
    return hintContextMenuable(qobject_cast<ContextMenuAction *>(action),
                               widget, proxies);
  default:
    logCritical << "BaseAction for HintMode" << action->mode
                << "not available in" << __PRETTY_FUNCTION__;
    Q_UNIMPLEMENTED();
  }
  Q_UNREACHABLE();
}

bool QWidgetActionProxyStatic::isContextMenuable(ContextMenuAction *action,
                                                 QWidget *widget) {
  // This leaves the uncommon possibilty that the client implemented a context
  // menu by implementing contextMenuEvent(QContextMenuEvent*) on the widget
  // subclass.
  return widget->contextMenuPolicy() == Qt::ActionsContextMenu ||
         widget->contextMenuPolicy() == Qt::CustomContextMenu;
}

static void hintGenericHelper(BaseAction *action, QWidget *widget,
                              QList<QWidgetActionProxy *> &proxies) {
  for (auto child : widget->children()) {
    QWidget *widget = qobject_cast<QWidget *>(child);
    if (widget && widget->isVisible() && widget->isEnabled()) {
      const QMetaObject *mo = widget->metaObject();
      // not interested in Tetradactyl's widgets
      if (mo == &HintLabel::staticMetaObject ||
          mo == &Overlay::staticMetaObject)
        continue;

      auto metadata = QWidgetActionProxy::getMetadataForMetaObject(mo);
      if (metadata.staticMethods->isHintableGeneric(action, widget)) {
        QWidgetActionProxy *proxy =
            QWidgetActionProxy::createForMetaObject(mo, widget);

        // fix this up
        proxies.append(proxy);
      }

      metadata.staticMethods->hintGeneric(action, widget, proxies);
    }
  }
}

void QWidgetActionProxyStatic::hintActivatable(
    ActivateAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  hintGenericHelper(action, widget, proxies);
}

void QWidgetActionProxyStatic::hintEditable(
    EditAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
  hintGenericHelper(action, widget, proxies);
}

void QWidgetActionProxyStatic::hintFocusable(
    FocusAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  hintGenericHelper(action, widget, proxies);
}

void QWidgetActionProxyStatic::hintYankable(
    YankAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
  hintGenericHelper(action, widget, proxies);
}

void QWidgetActionProxyStatic::hintContextMenuable(
    ContextMenuAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  hintGenericHelper(action, widget, proxies);
}

// QAbstractButtonActionProxy

bool QAbstractButtonActionProxy::activate(ActivateAction *action) {
  QOBJECT_CAST_ASSERT(QAbstractButton, widget);
  instance->setDown(true);
  instance->click();
  return true;
}

bool QAbstractButtonActionProxy::focus(FocusAction *action) {
  QOBJECT_CAST_ASSERT(QAbstractButton, widget);
  QWidgetActionProxy::focus(action);
  instance->setDown(true);
  return true;
}

bool QAbstractButtonActionProxy::yank(YankAction *action) {
  QOBJECT_CAST_ASSERT(QAbstractButton, widget);
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setText(instance->text());
  return true;
}

// QGroupBoxActionProxy

bool QGroupBoxActionProxyStatic::isActivatable(ActivateAction *action,
                                               QWidget *widget) {
  return qobject_cast<QGroupBox *>(widget)->isCheckable();
}

bool QGroupBoxActionProxy::activate(ActivateAction *action) {
  QOBJECT_CAST_ASSERT(QGroupBox, widget);
  instance->setChecked(!instance->isChecked());
  return true;
}

// QLabelActionProxy

bool QLabelActionProxy::yank(YankAction *action) {
  QOBJECT_CAST_ASSERT(QLabel, widget);
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setText(instance->text());
  return true;
}

// QStackedWidgetActionProxy

void stackedWidgetHintHelper(BaseAction *action, QWidget *widget,
                             QList<QWidgetActionProxy *> &proxies) {
  QOBJECT_CAST_ASSERT(QStackedWidget, widget);
  QWidget *childWidget = instance->currentWidget();
  if (childWidget && childWidget->isVisible() && childWidget->isEnabled()) {
    const QMetaObject *mo = childWidget->metaObject();
    // QStackedWidget chid should never be Tetradactyl widget
    Q_ASSERT(!isTetradactylMetaObject(mo));
    auto metadata = QWidgetActionProxy::getMetadataForMetaObject(mo);
    if (metadata.staticMethods->isHintableGeneric(action, widget)) {
      QWidgetActionProxy *proxy =
          QWidgetActionProxy::createForMetaObject(mo, widget);

      // fix this up
      proxies.append(proxy);
    }

    metadata.staticMethods->hintGeneric(action, widget, proxies);
  }
}

void QStackedWidgetActionProxyStatic::hintActivatable(
    ActivateAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  stackedWidgetHintHelper(action, widget, proxies);
}
void QStackedWidgetActionProxyStatic::hintEditable(
    EditAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
  stackedWidgetHintHelper(action, widget, proxies);
}
void QStackedWidgetActionProxyStatic::hintFocusable(
    FocusAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  stackedWidgetHintHelper(action, widget, proxies);
}
void QStackedWidgetActionProxyStatic::hintContextMenuable(
    ContextMenuAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  stackedWidgetHintHelper(action, widget, proxies);
}
void QStackedWidgetActionProxyStatic::hintYankable(
    YankAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
  stackedWidgetHintHelper(action, widget, proxies);
}

// QTabBarActionProxy

void QTabBarActionProxyStatic::hintActivatable(
    ActivateAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  QTabBar *instance = qobject_cast<QTabBar *>(widget);
  auto tabHintLocations = probeTabLocations(instance);

  // Via spy styles, we can snoop at most the visible number of tabs. This
  // limits the number of tabs we  action proxies we can create here.
  for (int idx = 0; idx != qMin(tabHintLocations.length(), instance->count());
       ++idx) {
    if (instance->isTabVisible(idx) && instance->isTabEnabled(idx)) {
      QTabBarActionProxy *proxy =
          new QTabBarActionProxy(idx, tabHintLocations.at(idx), instance);
      proxies.push_back(proxy

      );
    }
  }
}

// Very silly code that dynamically probes tab positions until we set up the
// style spy to supply this information.
QList<QPoint> QTabBarActionProxyStatic::probeTabLocations(QTabBar *bar) {
  QList<QPoint> points;
  const int step = 5;
  int currentIdx = 0;
  for (int x = 0; x < bar->rect().width(); x += step) {
    if (bar->tabAt(QPoint(x, 0)) == currentIdx) {
      int i = 0;
      for (; i != step + 1; ++i) {
        if (bar->tabAt(QPoint(x - i, 0)) != currentIdx)
          break;
      }
      points.push_back(QPoint(x - i + 1, 0));
      currentIdx++;
    }
  }
  return points;
}

bool QTabBarActionProxy::activate(ActivateAction *action) {
  QOBJECT_CAST_ASSERT(QTabBar, widget);
  instance->setCurrentIndex(tabIndex);
  return true;
}

} // namespace Tetradactyl
