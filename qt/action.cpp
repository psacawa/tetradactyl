// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractButton>
#include <QClipboard>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QGroupBox>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QWidget>
#include <cstring>
#include <qlist.h>
#include <qobject.h>

#include <algorithm>
#include <iterator>
#include <map>

#include "action.h"
#include "actionmacros.h"
#include "common.h"
#include "controller.h"
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
  case Menuable:
    return new MenuBarAction(winController);
  default:
    logCritical << "BaseAction for HintMode" << mode << "not available in"
                << __PRETTY_FUNCTION__;
    Q_UNIMPLEMENTED();
  }
  Q_UNREACHABLE();
}

void BaseAction::addNextStage(QWidget *root) {
  if (winController->findOverlayForWidget(root) == nullptr) {
    winController->addOverlay(root);
  }
  currentRoot = root;
  logInfo << "New stage of" << this << "based at root:" << root;
}

void BaseAction::accept(QWidgetActionProxy *proxy) {
  logInfo << "Accepting" << proxy;

  int widgetFinishesAction = proxy->actGeneric(this);
  emit winController->accepted(winController->currentHintMode, proxy->widget,
                               proxy->positionInWidget);
  if (widgetFinishesAction)
    finish();
}

void BaseAction::act() {
  QList<QWidgetActionProxy *> hintData;
  // get the hints
  const QMetaObject *targetMO = currentRoot->metaObject();
  auto metadata = QWidgetActionProxy::getMetadataForMetaObject(targetMO);
  metadata.staticMethods->hintGeneric(this, currentRoot, hintData);

  // Nothing hintable. End the action
  if (hintData.length() == 0) {
    logWarning << "Action hinting returned no hintable objects:" << this
               << currentRoot;
    finish();
    return;
  }
  Overlay *overlay = winController->findOverlayForWidget(currentRoot);
  HintGenerator hintStringGenerator(Controller::settings.hintChars,
                                    hintData.length());
  for (QWidgetActionProxy *actionProxy : hintData) {
    string hintStr = *hintStringGenerator;
    logDebug << "Hinting " << actionProxy->widget << " with "
             << QString::fromStdString(hintStr);
    overlay->addHint(QString::fromStdString(*hintStringGenerator), actionProxy);
    hintStringGenerator++;
  }
  overlay->resetSelection();
}

// ActivateAction

ActivateAction::ActivateAction(WindowController *controller)
    : BaseAction(controller) {
  mode = HintMode::Activatable;
  // Activation is by default immediately true, but there are exceptions for
  // e.g. QComboBox
}

// EditAction

EditAction::EditAction(WindowController *controller) : BaseAction(controller) {
  mode = HintMode::Editable;
}

// FocusAction

FocusAction::FocusAction(WindowController *controller)
    : BaseAction(controller) {
  mode = HintMode::Focusable;
}

// YankAction

YankAction::YankAction(WindowController *controller) : BaseAction(controller) {
  mode = HintMode::Yankable;
}

// MenuBarAction

MenuBarAction::MenuBarAction(WindowController *controller)
    : BaseAction(controller) {
  mode = HintMode::Menuable;
  // For menu actions
}

// ContextMenuAction

ContextMenuAction::ContextMenuAction(WindowController *controller)
    : BaseAction(controller) {
  mode = HintMode::Contextable;
  // For menu actions
}

// QList<QWidget *> ContextMenuAction::getHintables(QWidget *root) { return; }

// Basic criterion for hintability of an action
static bool isActionHintable(QAction *action) {
  return action->isEnabled() && action->isVisible() && !action->isSeparator();
}

// WIDGET PROXIES

#define PASTE(a, b) a##b
#define METADATA_REGISTRY_ENTRY(klass)                                         \
  {                                                                            \
    &klass::staticMetaObject, {                                                \
      &PASTE(klass, ActionProxy::staticMetaObject),                            \
          new PASTE(klass, ActionProxyStatic)                                  \
    }                                                                          \
  }

// A horrific static that enables us to use dynamic dispatch of static methods
// dependent on a widget's QMetaObject. We get the static methods and
// ActionProxies via this map.  Find a better way!
map<const QMetaObject *, WidgetHintingData> QWidgetMetadataRegistry = {
    METADATA_REGISTRY_ENTRY(QAbstractButton),
    METADATA_REGISTRY_ENTRY(QAbstractItemView),
    METADATA_REGISTRY_ENTRY(QComboBox),
    METADATA_REGISTRY_ENTRY(QGroupBox),
    METADATA_REGISTRY_ENTRY(QLabel),
    METADATA_REGISTRY_ENTRY(QLineEdit),
    METADATA_REGISTRY_ENTRY(QListView),
    METADATA_REGISTRY_ENTRY(QMenuBar),
    METADATA_REGISTRY_ENTRY(QMenu),
    METADATA_REGISTRY_ENTRY(QStackedWidget),
    METADATA_REGISTRY_ENTRY(QTabBar),
    METADATA_REGISTRY_ENTRY(QTableView),
    METADATA_REGISTRY_ENTRY(QWidget),
};

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
    auto msg = QString("No ActionProxy class found for QMetaObject of %1 which "
                       "inherits %2")
                   .arg(widgetMO->className())
                   .arg(iter->className());
    // only warn if there was another base Qt widget between this one QWidget
    if (strcmp((iter)->className(), "QWidget"))
      logWarning << qPrintable(msg);
    else
      logInfo << qPrintable(msg);
  }

  return metadata;
}

QWidgetActionProxy *
QWidgetActionProxy::createForMetaObject(const QMetaObject *widgetMO,
                                        QWidget *w) {
  WidgetHintingData metadata = getMetadataForMetaObject(widgetMO);
  QObject *obj = metadata.actionProxyMO->newInstance(Q_ARG(QWidget *, w));
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
  case Menuable:
    return this->menu(qobject_cast<MenuBarAction *>(action));
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
  case Menuable:
    return hintMenuable(qobject_cast<MenuBarAction *>(action), widget, proxies);
  default:
    logCritical << "BaseAction for HintMode" << action->mode
                << "not available in" << __PRETTY_FUNCTION__;
    Q_UNIMPLEMENTED();
  }
  Q_UNREACHABLE();
}

bool QWidgetActionProxyStatic::isContextMenuable(ContextMenuAction *action,
                                                 QWidget *widget) {
  // We must accept Qt::DefaultContextMenu because of the common case that the
  // client implemented a context menu by overriding
  // contextMenuEvent(QContextMenuEvent*) on the widget subclass. Use of this
  // technique is widespead. Unfortunately, since it's the default, it isn't
  // possible to distinguish this from the case where no such override exists
  // via normal means.
  return widget->contextMenuPolicy() == Qt::DefaultContextMenu ||
         widget->contextMenuPolicy() == Qt::ActionsContextMenu ||
         widget->contextMenuPolicy() == Qt::CustomContextMenu;
}

static void hintGenericHelper(BaseAction *action, QWidget *widget,
                              QList<QWidgetActionProxy *> &proxies) {
  for (auto child : widget->children()) {
    QWidget *widget = qobject_cast<QWidget *>(child);
    if (widget && widget->isVisible() && widget->isEnabled()) {
      const QMetaObject *mo = widget->metaObject();
      // not interested in Tetradactyl's widgets
      if (isTetradactylMetaObject(mo))
        continue;

      auto metadata = QWidgetActionProxy::getMetadataForMetaObject(mo);
      if (metadata.staticMethods->isHintableGeneric(action, widget)) {
        QWidgetActionProxy *proxy =
            QWidgetActionProxy::createForMetaObject(mo, widget);
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

// Jump directly to QMenuBar subclasses
void QWidgetActionProxyStatic::hintMenuable(
    MenuBarAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  QList<QMenuBar *> menuBars = widget->findChildren<QMenuBar *>();
  for (auto menuBar : menuBars) {
    if (menuBar->isVisible() && menuBar->isEnabled()) {
      QMenuBarActionProxyStatic menuBarStatic;
      menuBarStatic.hintMenuable(action, menuBar, proxies);
    }
  }
}

bool QWidgetActionProxy::contextMenu(ContextMenuAction *action) {
  Qt::ContextMenuPolicy policy = widget->contextMenuPolicy();
  const QPoint globalPos = widget->mapTo(widget->window(), QPoint(0, 0));
  switch (policy) {
  case Qt::DefaultContextMenu: {
    // Pray to God this reaches the right widget.
    QContextMenuEvent cmev(QContextMenuEvent::Mouse, QPoint(0, 0), globalPos);
    logInfo << "Sending QContextMenuEvent" << &cmev << "to receiver" << widget;
    QCoreApplication::instance()->sendEvent(widget, &cmev);
    break;
  }
  case Qt::ActionsContextMenu: {
    QMenu::exec(widget->actions(), globalPos, nullptr, widget);
    break;
  }
  case Qt::CustomContextMenu: {
    // TODO 16/09/20 psacawa: Special logic must  be added for instances of
    // QAbstractScrollArea
    // cf: https://doc.qt.io/qt-6/qwidget.html#customContextMenuRequested
    emit widget->customContextMenuRequested(QPoint(0, 0));
    break;
  }
  default:
    logWarning << "Unexpected ContextMenuPolicy" << policy << "for widget"
               << widget;
    return false;
  }
  return true;
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

// QComboBoxActionProxy

bool QComboBoxActionProxyStatic::isActivatable(ActivateAction *action,
                                               QWidget *widget) {
  QOBJECT_CAST_ASSERT(QComboBox, widget);
  return instance->count() > 0;
}

bool QComboBoxActionProxyStatic::isEditable(EditAction *action,
                                            QWidget *widget) {
  QOBJECT_CAST_ASSERT(QComboBox, widget);
  return instance->isEditable();
}

bool QComboBoxActionProxy::activate(ActivateAction *action) {
  QOBJECT_CAST_ASSERT(QComboBox, widget);
  instance->showPopup();
  // action->addNextStage(instance);
  return false;
}

bool QComboBoxActionProxy::yank(YankAction *action) {
  QOBJECT_CAST_ASSERT(QComboBox, widget);
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setText(instance->currentText());
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

// QMenuBarActionProxy

void QMenuBarActionProxyStatic::hintMenuable(
    MenuBarAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  QOBJECT_CAST_ASSERT(QMenuBar, widget);
  for (auto menuAction : instance->actions()) {
    if (isActionHintable(menuAction)) {
      QRect geometry = instance->actionGeometry(menuAction);
      QMenuBarActionProxy *proxy =
          new QMenuBarActionProxy(instance, geometry.topLeft(), menuAction);
      proxies.append(proxy);
    }
  }
}

// Helper that should be in Qt itself.
// FIXME 20/09/20 psacawa: slow as shit
QMenu *getMenuForMenuBarAction(QWidget *w, QAction *action) {
  // Menus in a QMenuBar don't actually need to be it's
  // descendants, e.g. if added with the addMenu API, so you can't use
  // necessarily use findChildren to recover them.
  // Make a faster implementation that registers the  correspondence action ->
  // menu in the windowController.
  for (auto w : qApp->allWidgets()) {
    QMenu *menu = qobject_cast<QMenu *>(w);
    if (menu && menu->menuAction() == action) {
      return menu;
    }
  }
  return nullptr;
}

bool QMenuBarActionProxy::menu(MenuBarAction *tetradactylAction) {
  QOBJECT_CAST_ASSERT(QMenuBar, widget);
  QMenu *menu = getMenuForMenuBarAction(instance, menuAction);
  Q_ASSERT(menu != nullptr);

  QPoint pos = menu->mapToGlobal(QPoint(0, 0));
  logInfo << "Opening menu of menu bar" << menuAction << instance << pos;
  QPoint globalPos = instance->window()->mapToGlobal(QPoint(0, 0));
  QPoint positionInWindow = instance->actionGeometry(menuAction).bottomLeft();

  menu->popup(globalPos + positionInWindow);

  tetradactylAction->menusToClose.append(menu);
  tetradactylAction->addNextStage(menu);
  return false;
}

// QMenuActionProxy

void QMenuActionProxyStatic::hintMenuable(
    MenuBarAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  QOBJECT_CAST_ASSERT(QMenu, widget);
  for (auto menuAction : instance->actions()) {
    if (isActionHintable(menuAction)) {
      QRect geometry = instance->actionGeometry(menuAction);
      QMenuActionProxy *proxy =
          new QMenuActionProxy(instance, geometry.topLeft(), menuAction);
      proxies.append(proxy);
    }
  }
}

bool QMenuActionProxy::menu(MenuBarAction *tetradactylAction) {
  QOBJECT_CAST_ASSERT(QMenu, widget);
  QMenu *submenu = getMenuForMenuBarAction(instance, menuAction);
  if (submenu != nullptr && submenu->actions().length() > 0) {
    // action has submenu
    logInfo << __PRETTY_FUNCTION__ << submenu << menuAction;
    QPoint pos = submenu->mapToGlobal(QPoint(0, 0));
    logInfo << "Opening submenu of submenu" << menuAction << instance << pos;
    QPoint globalPos = instance->window()->mapToGlobal(QPoint(0, 0));
    QPoint positionInWindow =
        instance->actionGeometry(menuAction).bottomRight();
    submenu->popup(globalPos + positionInWindow);
    tetradactylAction->menusToClose.append(submenu);
    tetradactylAction->addNextStage(submenu);
    return false;
  } else {
    // trigger final action
    logInfo << "activating" << instance << menuAction;
    menuAction->trigger();

    // Hide all the previously opened menus
    for (auto menu : tetradactylAction->menusToClose)
      menu->hide();

    tetradactylAction->finish();
    return true;
    ;
  }
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

void QTabBarActionProxyStatic::hintHelper(
    BaseAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
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

void QTabBarActionProxyStatic::hintActivatable(
    ActivateAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  QTabBarActionProxyStatic::hintHelper(action, widget, proxies);
}

void QTabBarActionProxyStatic::hintYankable(
    YankAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
  QTabBarActionProxyStatic::hintHelper(action, widget, proxies);
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

bool QTabBarActionProxy::yank(YankAction *action) {
  QOBJECT_CAST_ASSERT(QTabBar, widget);
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setText(instance->tabText(tabIndex));
  return true;
}

} // namespace Tetradactyl
