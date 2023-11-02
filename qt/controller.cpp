// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QKeySequence>
#include <QLineEdit>
#include <QList>
#include <QLoggingCategory>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPointer>
#include <QShortcut>
#include <QString>
#include <QTextEdit>
#include <QTimer>
#include <QWindow>

#include <algorithm>
#include <csignal>
#include <iterator>
#include <vector>

#include "action.h"
#include "commandline.h"
#include "commands.h"
#include "controller.h"
#include "filter.h"
#include "hint.h"
#include "logging.h"
#include "overlay.h"
#include "probe.h"

LOGGING_CATEGORY_COLOR("tetradactyl.controller", Qt::blue);

// using namespace Qt::Literals::StringLiterals;

using Qt::WindowType;
using std::copy_if;
using std::size_t;

namespace Tetradactyl {

const std::map<HintMode, vector<const QMetaObject *>> hintableMetaObjects = {
    {Activatable, {&QAbstractButton::staticMetaObject}},
    {Editable, {&QLineEdit::staticMetaObject, &QTextEdit::staticMetaObject}},
    {Focusable,
     {&QLineEdit::staticMetaObject, &QAbstractButton::staticMetaObject}},
    {Yankable, {&QLabel::staticMetaObject}}};

static ControllerSettings baseTestSettings() {
  return ControllerSettings{
      .hintChars = "ASDFJKL",
      .autoAcceptUniqueHint = true,
      .highlightAcceptedHint = true,
      .highlightAcceptedHintMs = 400,
      .passthroughKeyboardInput = true,
      .resetModeAfterFocusChange = true,
      .keymap = {.activate = QKeySequence(Qt::Key_F),
                 .cancel = QKeySequence(Qt::Key_Escape),
                 .edit = QKeySequence(Qt::Key_G, Qt::Key_I),
                 .focus = QKeySequence(Qt::Key_Semicolon),
                 .yank = QKeySequence(Qt::Key_Y),
                 .activateMenu = QKeySequence(Qt::Key_M),
                 .activateContext = QKeySequence(Qt::Key_C),
                 .upScroll = QKeySequence(Qt::Key_K),
                 .downScroll = QKeySequence(Qt::Key_J),
                 .focusPrompt = QKeySequence(Qt::Key_Colon)},
  };
}

struct ControllerSettings Controller::settings = baseTestSettings();

QString Controller::stylesheet = "";

Controller::Controller() : resetPending(false) {
  Q_ASSERT(self == nullptr);
  // Having the application as the parent allows a tool like  gammaray to find
  // it on startup
  setParent(qApp);

  QTimer *timer = new QTimer;
  timer->setInterval(2000);

  connect(timer, &QTimer::timeout, [] {
    for (auto &win : tetradactyl->windows()) {
      logInfo << win << win->activeOverlay();
    }
  });
  // timer->start();

  Controller::stylesheet = fetchStylesheet();
  qApp->setStyleSheet(Controller::stylesheet);
  qApp->installEventFilter(new Tetradactyl::PrintFilter);
  qApp->installEventFilter(this);

  if (settings.resetModeAfterFocusChange) {
    connect(qApp, &QApplication::focusChanged, this,
            &Controller::resetModeAfterFocusChange);
    connect(qApp, &QApplication::focusWindowChanged, this,
            &Controller::resetModeAfterFocusWindowChanged);
  }

  emit started();
}

Controller::~Controller() {
  cleanupWindows();
  if (qApp)
    qApp->removeEventFilter(this);
  self = nullptr;
}

void Controller::executeCommand(QString cmdline) {
  QList<QString> argv = cmdline.split(' ');

  // parse
  qInfo() << "executing: " << argv[0];
  runCommand(argv);
}

void Controller::resetWindows() {
  Q_ASSERT(tetradactyl);
  Q_ASSERT(!resetPending);
  logInfo << "Resetting WindowControllers";
  resetPending = true;
  cleanupWindows();
  initWindows();
  resetPending = false;
  emit reset();
}

void Controller::cleanupWindows() {
  if (self == nullptr) {
    // controller already deleted
    return;
  }
  while (!windowControllers.isEmpty()) {
    WindowController *winc = windowControllers.last();
    delete winc;
    windowControllers.pop_back();
  }
}

void Controller::createController() {
  logInfo << "Creating Tetradactyl controller";
  self = new Controller();

  self->initWindows();

  // Initially defocus input widgets. This assumes the top-level widget is not
  // e.g. QLineEdit
  for (auto widget : qApp->topLevelWidgets()) {
    widget->clearFocus();
  }
}

WindowController *Controller::findControllerForWidget(QWidget *widget) {
  if (widget == nullptr)
    return nullptr;

  for (auto winController : windowControllers) {
    // Two cases here: either the widget itself (potentially itself) has the
    // WindowController, or itself it's a popup of sorts (QMenu/QComboBox popup)
    // whose nativeParentWidget has the WindowController
    QWidget *target = winController->target();
    QWidget *iter = widget;
    while (iter != nullptr) {
      if (iter == target) {
        return winController;
      }
      iter = iter->nativeParentWidget();
    }
  }
  return nullptr;
}

void Controller::initWindows() {
  QList<QWidget *> topLevelWidgets = qApp->allWidgets();
  QList<QWidget *> tetradactylWindows;
  copy_if(topLevelWidgets.begin(), topLevelWidgets.end(),
          std::back_inserter(tetradactylWindows), isTetradactylWindow);
  for (auto &widget : tetradactylWindows) {
    attachControllerToWindow(widget);
  }
}

// Check if the widget doesn't already have an
// attached WindowController. If so, create a WindowController. NB it's
// necessary for invisible window to have active controller in the test suite.
// @pre: isTetradactylWindow(widget)
void Controller::attachControllerToWindow(QWidget *widget) {
  Q_ASSERT(isTetradactylWindow(widget));
  if (findControllerForWidget(widget) == nullptr) {
    logInfo << "Attaching Tetradactyl to" << widget;
    self->windowControllers.append(new WindowController(widget, this));
  }
}

bool Controller::eventFilter(QObject *receiver, QEvent *ev) {
  QEvent::Type type = ev->type();
  if (receiver->isWidgetType()) {
    QWidget *widget = qobject_cast<QWidget *>(receiver);
    if (type == QEvent::KeyPress) {
      QKeyEvent *kev = static_cast<QKeyEvent *>(ev);
      WindowController *windowController = findControllerForWidget(widget);
      logInfo << "sending KeyPress event to" << windowController;
      if (windowController) {
        bool accepted = windowController->earlyKeyEventFilter(kev);
        return accepted;
      }
    } else if (type == QEvent::ParentChange && !resetPending &&
               objProbe->isClientWidget(widget)) {
      // POLICY TEST: Reset windows on QWidget reparented
      logInfo << "ParentChange" << type << "for" << receiver
              << "Resetting Controller";
      QTimer::singleShot(0, [] { tetradactyl->resetWindows(); });
    }
  }
  return false;
}

void Controller::routeNewlyCreatedObject(QObject *obj) {
  const QMetaObject *mo = obj->metaObject();
  if (!mo->inherits(&QWidget::staticMetaObject)) {
    return;
  }
  QWidget *widget = qobject_cast<QWidget *>(obj);

  // Create overlays for some types of QObject
  if (mo->inherits(&QMenu::staticMetaObject)) {

    // Object at this point may be a descendant of a window with a Tetradactyl
    // controller, but it may not be.
    QWidget *win = widget->window();
    WindowController *winController = findControllerForWidget(win);
    if (winController) {
      winController->addOverlay(widget);
    } else {
      // TODO 11/09/20 psacawa: this is the case of a QMenu etc. added. We need
      // to add the overlay if the controlling window is none, or set up a
      // notification mechanism if it isn't.
    }
  }
  // TODO 09/09/20 psacawa: add WindowController overlay or wait for show
  // event as necessary
}

// Adjust controller states in response to QApplication::focusChanged. The
// cases:
//
// i) clearFocus was called (now == nullptr).
// ii) setFocus was called from unfocussed state (old == nullptr)
// iii) regulat change between widgets in one window (tab/shitf+tab)
// iv) change between widgets with different controllers. Eithr old or now may
// be null
// v) part of a multistep action: QComboBox, QMenu  activate, etc...
//
// Generally, we want to fix controller modes for the current focus, but permit
// different windowControllers to  have their own state. It's in general
// half-baked idea to use this slot to control much of anything.
void Controller::resetModeAfterFocusChange(QWidget *old, QWidget *now) {
  logInfo << __FUNCTION__ << old << now;
  WindowController *oldWindowController = findControllerForWidget(old);
  WindowController *nowWindowController = findControllerForWidget(now);

  // Case where old, now both have the same controller. This is the case in
  // multistep hinting
  if (oldWindowController && oldWindowController == nowWindowController) {
    if (inputModeWhenWidgetFocussed(now)) {
      nowWindowController->setControllerMode(Input);
    }
    return;
  }

  if (old != nullptr) {
    if (oldWindowController) {
      oldWindowController->setControllerMode(Normal);
    }
  }
  if (now != nullptr) {
    if (nowWindowController && inputModeWhenWidgetFocussed(now)) {
      nowWindowController->setControllerMode(Input);
    } else {
      // Find out if this was a newly created tetradactylWindow, if so create
      // the controller.
      if (isTetradactylWindow(now)) {
        logInfo << "Adding new WindowController to" << now;
        tetradactyl->resetWindows();
      }
    }
  }
}

// The active QWindow has changed. This can happen as part of a multi-step hint
// process, or upon cancellation of such process via a mouse click, or
// client-driven refocus. We must identify these cases and update case
// accordingly.
void Controller::resetModeAfterFocusWindowChanged(QWindow *focusWindow) {
  logInfo << __FUNCTION__ << focusWindow;

  if (focusWindow == nullptr)
    return;

  QWidget *focusWidget = qApp->focusWidget();
  // untrue that focusWidget != nullptr here
  // untrue even that qApp->focusWidget() ==
  // getToplevelWidgetForWindow(focusWindow) !
  QWidget *topLevelWidget = getToplevelWidgetForWindow(focusWindow);

  // handle Controller state changes for focusWidget
  WindowController *controller = findControllerForWidget(topLevelWidget);
  if (controller == nullptr)
    return;

  BaseAction *action = controller->currentAction();
  if (action != nullptr && action->currentRoot() == topLevelWidget) {
    // this convinces us that we are in a multi-step hint process
    logInfo << "No mode change in " << __FUNCTION__ << " - In hint mode at"
            << topLevelWidget;
    return;
  }

  ControllerMode nowMode =
      (focusWidget && inputModeWhenWidgetFocussed(focusWidget)) ? Input
                                                                : Normal;
  controller->setControllerMode(nowMode);
}

Controller *Controller::self = nullptr;

QDebug operator<<(QDebug debug, const Controller *controller) {
  QDebugStateSaver saver(debug);
  debug << "Controller("
        << "#WindowController: " << controller->windowControllers.length()
        << ")";
  return debug;
}

// while key handling in ControllerMode::Normal is being settlied, we use Qt
// native shortcuts. This let's us use e.g. multi step key sequences, but it's
// an issue and the clients own shortcuts, but it's a problem if there is an
// shortcut collision.
void WindowController::initializeShortcuts() {
  ControllerKeymap &keymap = Controller::instance()->settings.keymap;
  QShortcut *activateShortcut =
      new QShortcut(keymap.activate, p_target, [this] { hint(); });
  QShortcut *editShortcut = new QShortcut(keymap.edit, p_target,
                                          [this] { hint(HintMode::Editable); });
  QShortcut *yankShortcut = new QShortcut(keymap.yank, p_target,
                                          [this] { hint(HintMode::Yankable); });
  QShortcut *focusShortcut = new QShortcut(
      keymap.focus, p_target, [this] { hint(HintMode::Focusable); });
  QShortcut *contextMenuShortcut =
      new QShortcut(keymap.activateContext, p_target,
                    [this] { hint(HintMode::Contextable); });
  QShortcut *menuShortcut = new QShortcut(keymap.activateMenu, p_target,
                                          [this] { hint(HintMode::Menuable); });
  QShortcut *cancelShortcut =
      new QShortcut(keymap.cancel, p_target, [this] { cancel(); });
  QShortcut *focusPromptShortcut = new QShortcut(
      keymap.focusPrompt, p_target, [this] { this->focusPrompt(); });
  shortcuts.append(activateShortcut);
  shortcuts.append(editShortcut);
  shortcuts.append(yankShortcut);
  shortcuts.append(focusShortcut);
  shortcuts.append(contextMenuShortcut);
  shortcuts.append(menuShortcut);
  shortcuts.append(cancelShortcut);
  shortcuts.append(focusPromptShortcut);
}

WindowController::WindowController(QWidget *_target, QObject *parent = nullptr)
    : QObject(parent), p_currentAction(nullptr), p_target(_target) {

  Q_ASSERT(parent);
  initializeShortcuts();
  initializeOverlays();
  Q_ASSERT(p_overlays.length() > 0);
  logInfo << "WindowController installs eventFilter on" << this;
  p_target->installEventFilter(this);
}

WindowController::~WindowController() {
  for (auto &overlay : p_overlays) {
    if (overlay)
      delete overlay;
  }
  for (auto shortcut : shortcuts) {
    if (shortcut)
      delete shortcut;
  }
}

// Find overlay which has the widget as *descendant*, not just as a child,
// which would be easier
Overlay *WindowController::findOverlayForWidget(QWidget *widget) {
  for (auto overlay : p_overlays) {
    if (overlay->parentWidget()->isAncestorOf(widget))
      return overlay;
  }
  return nullptr;
}

static QList<const QMetaObject *> inputModeWidgetTypes = {
    &QLineEdit::staticMetaObject, &QTextEdit::staticMetaObject};

// @pre: w != nullptr
bool inputModeWhenWidgetFocussed(QWidget *w) {
  Q_ASSERT(w != nullptr);
  const QMetaObject *const mo = w->metaObject();
  auto metadata = getMetadataForMetaObject(mo);
  return metadata.staticMethods->isEditable(nullptr, w);
}

/*
 * Filter QKeyEvents before the receiver widgets get a change to act on them.
 * Called from the application eventFilter.
 */
bool WindowController::earlyKeyEventFilter(QKeyEvent *kev) {
  logDebug << __FUNCTION__ << kev;
  auto type = kev->type();
  if (type == QEvent::KeyPress) {
    switch (p_controllerMode) {

    case ControllerMode::Normal: {
      // Need to handle main shortcut logic here
      switch (kev->key()) {
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
        // focus next/prev widget
      default:
        break;
      }
      break;
    }

    case ControllerMode::Hint: {
      switch (kev->key()) {
      case Qt::Key_Escape:
        cancel();
        return true;
      case Qt::Key_Return:
        acceptCurrent();
        return true;
      case Qt::Key_Backspace:
        popKey();
        return true;
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
        // activeOverlay()->nextHint(!(kev->modifiers() & Qt::ShiftModifier));
        activeOverlay()->nextHint((kev->key() == Qt::Key_Tab));
        // activeOverlay()->nextHint((true));
        return true;
      }
      if ((kev->key() >= 'A') && (kev->key() <= 'Z')) {
        // std::isalpha doesn't work here
        logDebug << kev->key();
        pushKey(kev->key());
        return true;
      }
      break;
    }

    case ControllerMode::Input:
      if (kev->key() == Qt::Key_Escape) {
        escapeInput();
        return true;
      }

    case ControllerMode::Ignore:
      break;
    }
  } else if (type == QEvent::KeyRelease) {
    // anything to do here?
  }
  return false;
}

bool WindowController::eventFilter(QObject *obj, QEvent *ev) {
  auto type = ev->type();
  switch (type) {
  case QEvent::Resize: {
    QWidget *widget = qobject_cast<QWidget *>(obj);
    Overlay *overlay = findOverlayForWidget(widget);
    if (overlay)
      overlay->layout()->update();
    break;
  }
  default:
    break;
  }
  return false;
}

// Current strategy: just search for the overlayable classes under the
// target
void WindowController::initializeOverlays() {
  QList<QWidget *> widgets = p_target->findChildren<QWidget *>();
  for (auto widget : qApp->topLevelWidgets()) {
    if (isDescendantOf(widget, p_target)) {
      tryAttachController(widget);
    }
  }
}

// The main overlay is considered to the be the one that has the others as
// descendants.
Overlay *WindowController::mainOverlay() {
  for (auto overlay : p_overlays) {
    if (overlay->host() == p_target ||
        overlay->host()->parentWidget() == p_target) {
      return overlay;
    }
  }
  logCritical << "No Main overlay found for " << this
              << " with the following available: " << p_overlays;
  return p_overlays.at(0);
}

// As with attachControllerToWindow, can be called to attach to existing
// overlayable widget, or in response to one being created.
void WindowController::tryAttachController(QWidget *target) {
  // TODO 09/09/20 psacawa: add checks for both cases
  if (target != nullptr) {
    addOverlay(target);
  }
}

Overlay *WindowController::activeOverlay() {
  QWidget *activeWidget = qApp->activePopupWidget();
  if (!activeWidget)
    activeWidget = qApp->focusWidget();

  Overlay *ret = findOverlayForWidget(activeWidget);
  if (ret == nullptr)
    ret = mainOverlay();
  return ret;
}

void WindowController::addOverlay(QWidget *target) {
  // exclude WindowType::Popup to get rid of QMenus
  bool isMainWindow = isTetradactylWindow(target);
  Overlay *overlay = new Overlay(this, target, isMainWindow);
  p_overlays.append(overlay);
}

void WindowController::removeOverlay(Overlay *overlay, bool fromSignal) {
  if (!fromSignal) {
    delete overlay;
    return;
  }
  int idx = p_overlays.indexOf(overlay);
  if (idx >= 0) {
    p_overlays.removeAt(idx);
  }
}

void WindowController::hint(HintMode hintMode) {
  if (!(controllerMode() == ControllerMode::Normal)) {
    logWarning << __PRETTY_FUNCTION__ << "from" << controllerMode();
    return;
  }
  logInfo << "Hinting in " << hintMode << "at" << target();
  hintBuffer = "";
  p_currentAction = BaseAction::createActionByHintMode(hintMode, this);
  p_currentAction->act();

  // Action may terminate immediately if there are no hints made
  // TODO 22/09/20 psacawa: consolidate with the cleanupWindows code in accept()
  if (p_currentAction->isDone()) {
    cleanupAction();
    return;
  }

  // We say that no controller state change occured if the the action ended
  // immediately.
  setControllerMode(Hint);

  setCurrentHintMode(hintMode);
  emit hinted(hintMode);
}

void WindowController::acceptCurrent() {
  HintLabel *hint = activeOverlay()->selectedHint();
  if (hint == nullptr) {
    logWarning << "Active overlay has no selected hint";
    return;
  }
  QWidgetActionProxy *proxy = hint->proxy;
  accept(proxy);
}

// Accept *one* stage of a potentially multi-stage action.
void WindowController::accept(QWidgetActionProxy *widgetProxy) {
  if (!(controllerMode() == ControllerMode::Hint)) {
    logWarning << __PRETTY_FUNCTION__ << "from" << controllerMode();
    return;
  }
  QWidget *w = widgetProxy->widget;
  logInfo << "Accepted " << w << "at" << widgetProxy->positionInWidget << "in"
          << p_currentHintMode;
  if (Controller::settings.highlightAcceptedHint) {
    QPointer<HintLabel> acceptedHint = activeOverlay()->selectedHint();
    activeOverlay()->popHint(acceptedHint);
    acceptedHint->setParent(activeOverlay()->parentWidget());
    acceptedHint->show();
    QTimer::singleShot(Controller::settings.highlightAcceptedHintMs,
                       [acceptedHint]() {
                         // make sure the hint still exist
                         if (acceptedHint) {
                           delete acceptedHint;
                         }
                       });
  }
  p_currentAction->accept(widgetProxy);
  cleanupHints();
  if (p_currentAction->isDone()) {
    setControllerMode(p_currentAction->controllerModeAfterSuccess());
    cleanupAction();
    emit hintingFinished(true);
    // The controller mode is not reset here but rather in the Controller's
    // QApplication::focusChanged signal handler
  } else {
    p_currentAction->act();
  }
}

void WindowController::escapeInput() {
  Q_ASSERT(controllerMode() == Input);
  QWidget *focussedWidget = qApp->focusWidget();
  if (!focussedWidget)
    logWarning << "in" << ControllerMode::Input << "without a focussed widget";
  focussedWidget->clearFocus();
  setControllerMode(Normal);
}

void WindowController::pushKey(char ch) {
  if (!(controllerMode() == ControllerMode::Hint)) {
    logWarning << __PRETTY_FUNCTION__ << "from" << controllerMode();
    return;
  }
  hintBuffer += ch;
  logInfo << "pushKey" << hintBuffer << activeOverlay()->parentWidget();
  int numVisibleHints = activeOverlay()->updateHints(hintBuffer);
  if (numVisibleHints == 1 && Controller::settings.autoAcceptUniqueHint) {
    acceptCurrent();
  } else if (numVisibleHints == 0) {
    cancel();
  }
}

void WindowController::popKey() {
  if (!(controllerMode() == ControllerMode::Hint)) {
    logWarning << __PRETTY_FUNCTION__ << "from" << controllerMode();
    return;
  }
  hintBuffer.remove(hintBuffer.length() - 1, 1);
  activeOverlay()->updateHints(hintBuffer);
}

void WindowController::focusPrompt() { mainOverlay()->commandLine()->open(); }

QWidget *WindowController::target() { return this->p_target; }

void WindowController::cleanupHints() {
  logDebug << __PRETTY_FUNCTION__;
  for (auto overlay : p_overlays)
    if (overlay)
      overlay->clear();
    else
      logWarning << "WindowController had overlay deleted:" << this;

  hintBuffer = "";
}

void WindowController::cleanupAction() {
  delete p_currentAction;
  p_currentAction = nullptr;
}

void WindowController::cancel() {
  if (!(controllerMode() == ControllerMode::Hint)) {
    return;
  }
  cleanupHints();
  emit cancelled(p_currentHintMode);
  emit hintingFinished(false);
  setControllerMode(Normal);
}

// For now the  hinting proceed only by type: QObjects whose meta-object
// are right get hinted. However, to support clients with more complex
// handling of events, etc., we will want a more dynamic approach.
QList<QWidget *> WindowController::getHintables(HintMode hintMode) {
  QList<QWidget *> descendants = p_target->findChildren<QWidget *>();
  vector<QWidget *> hintables;
  vector<const QMetaObject *> metaObjects;
  try {
    metaObjects = hintableMetaObjects.at(hintMode);
  } catch (std::out_of_range &e) {
    logWarning << "unsupported" << hintMode;
    return {};
  }

  std::copy_if(descendants.begin(), descendants.end(),
               std::back_inserter(hintables), [=](QWidget *widget) {
                 if (!widget->isEnabled()) {
                   return false;
                 }
                 for (auto mo : metaObjects) {
                   if (mo->cast(widget) != nullptr) {
                     return true;
                   }
                 }
                 return false;
               });

  return QList<QWidget *>(hintables.begin(), hintables.end());
}

void WindowController::setControllerMode(ControllerMode mode) {
  logInfo << __FUNCTION__ << "from" << p_controllerMode << "to" << mode;
  bool changed = mode != p_controllerMode;
  p_controllerMode = mode;
  for (auto sc : shortcuts)
    sc->setEnabled(mode == Normal);

  if (!changed)
    return;

  if (mode != Hint) {
    cleanupHints();
  }

  emit modeChanged(mode);
}

QDebug operator<<(QDebug debug, const WindowController *controller) {
  QDebugStateSaver saver(debug);
  debug.nospace();

  debug << WindowController::staticMetaObject.className();
  if (!controller) {
    debug << "(0x0)";
  } else {
    debug << "(" << (void *)controller << " " << controller->p_target << ", "
          << controller->p_controllerMode;
    if (controller->p_controllerMode == ControllerMode::Hint)
      debug << ", " << controller->p_currentHintMode;
    if (debug.verbosity() > QDebug::DefaultVerbosity) {
      debug << "overlays=" << controller->p_overlays.length() << ": ";
      for (int i = 0; i != controller->p_overlays.length(); i++) {
        auto overlay = controller->p_overlays.at(i);
        if (i != 0)
          debug << ", ";
        debug << overlay->parentWidget();
      }
    }
    debug << ")";
  }
  return debug;
}

HintGenerator::HintGenerator(const char *_hintChars, size_t size)
    : hintChars(_hintChars), pos(0) {
  lengthOfHints = 0;
  // find the minimum lengthOfHints to be able to generate enough hint
  // strings
  for (size_t numHintsGenerated = 1; numHintsGenerated < size;
       numHintsGenerated *= hintChars.size()) {
    lengthOfHints++;
  }
  // edge case
  if (lengthOfHints == 0) {
    lengthOfHints = 1;
  }
}

HintGenerator::~HintGenerator() {}

string HintGenerator::operator*() { return generateHint(); }

HintGenerator &HintGenerator::operator++() {
  pos++;
  return *this;
}

HintGenerator &HintGenerator::operator++(int _) {
  operator++();
  return *this;
}

string HintGenerator::generateHint() {
  string ret(lengthOfHints, ' ');
  size_t tmp = pos;
  for (ssize_t idx = lengthOfHints - 1; idx >= 0; idx--) {
    ret[idx] = hintChars[tmp % hintChars.length()];
    tmp /= hintChars.length();
  }
  return ret;
}

QString fetchStylesheet() {
  QFile hintsCss(":/tetradactyl/hints.css");
  hintsCss.open(QIODevice::ReadOnly);
  QString ret = hintsCss.readAll();
  return ret;
}

} // namespace Tetradactyl
