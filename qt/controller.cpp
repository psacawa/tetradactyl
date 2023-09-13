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
#include <QShortcut>
#include <QTextEdit>
#include <QTimer>
#include <QWindow>

#include <fmt/core.h>

#include <algorithm>
#include <iterator>
#include <qdebug.h>
#include <qglobal.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qt6/QtCore/qmap.h>
#include <qvariant.h>
#include <vector>

#include "controller.h"
#include "filter.h"
#include "hint.h"
#include "logging.h"
#include "overlay.h"

LOGGING_CATEGORY_COLOR("tetradactyl.controller", Qt::blue);

using Qt::WindowType;
using std::size_t;

namespace Tetradactyl {

static bool isDescendantOf(QObject *descendant, QObject *ancestor) {
  for (; descendant != nullptr; descendant = descendant->parent()) {
    if (descendant == ancestor)
      return true;
  }
  return false;
}

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
      .highlightAcceptedHintMs = 1000,
      .passthroughKeyboardInput = true,
      .keymap = {.activate = QKeySequence(Qt::Key_F),
                 .cancel = QKeySequence(Qt::Key_Escape),
                 .edit = QKeySequence(Qt::Key_G, Qt::Key_I),
                 .focus = QKeySequence(Qt::Key_Semicolon),
                 .yank = QKeySequence(Qt::Key_Y),
                 .activateMenu = QKeySequence(Qt::Key_M),
                 .activateContext = QKeySequence(Qt::Key_G, Qt::Key_C),
                 .upScroll = QKeySequence(Qt::Key_K),
                 .downScroll = QKeySequence(Qt::Key_J)},
  };
}

struct ControllerSettings Controller::settings = baseTestSettings();

QString Controller::stylesheet = "";

Controller::Controller() {
  Q_ASSERT(self == nullptr);
  Controller::stylesheet = fetchStylesheet();
  qApp->setStyleSheet(Controller::stylesheet);

  qApp->installEventFilter(new Tetradactyl::PrintFilter);
  qApp->installEventFilter(this);
}

Controller::~Controller() {
  for (auto &WindowController : windowControllers) {
    delete WindowController;
  }
  self = nullptr;
}

void Controller::createController() {
  logInfo << "Creating Tetradactyl controller";
  self = new Controller();

  self->attachToExistingWindows();

  // Initially defocus input widgets. This assumes the tol-level widget is not
  // e.g. QLineEdit
  for (auto widget : qApp->topLevelWidgets()) {
    widget->clearFocus();
  }
}

WindowController *Controller::findControllerForWidget(QWidget *widget) {
  for (auto winController : windowControllers) {
    if (widget->window() == winController->target())
      return winController;
  }
  return nullptr;
}

void Controller::attachToExistingWindows() {
  QList<QWidget *> topelevelWidgets = qApp->topLevelWidgets();
  for (auto &widget : topelevelWidgets) {
    // QTimer::singleShot(0, [widget] { self->tryAttachToWindow(widget); });
    tryAttachToWindow(widget);
  }
}

// Check if the widget is a window,not a popup, and doesn't alredy have an
// attached WindowController. If so, create a WindowController. NB it's
// necessary for invisible window to have active controller in the test suite.
void Controller::tryAttachToWindow(QWidget *widget) {
  // Make sure no to attach WindowController to things with the popup bit
  // set, such as  QMenu (in menu bar and context menu)
  if (widget->isWindow() &&
      !(widget->windowFlags() & WindowType::Popup & ~WindowType::Window) &&
      findControllerForWidget(widget) == nullptr) {
    logInfo << "Attaching Tetradactyl to" << widget;
    self->windowControllers.append(new WindowController(widget, this));
    // }
  }
}

// TODO 09/09/20 psacawa: test it works
bool Controller::eventFilter(QObject *obj, QEvent *ev) {
  QEvent::Type type = ev->type();
  QWidget *widget = qobject_cast<QWidget *>(obj);
  if (widget) {
    if (type == QEvent::Show) {
      tryAttachToWindow(widget);
    } else if (type == QEvent::KeyPress) {
      QKeyEvent *kev = static_cast<QKeyEvent *>(ev);
      WindowController *windowController = findControllerForWidget(widget);
      if (windowController) {
        bool accepted = windowController->earlyKeyEventFilter(kev);
        return accepted;
      }
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

Controller *Controller::self = nullptr;

void WindowController::initializeShortcuts() {
  ControllerKeymap &keymap = controller->settings.keymap;
  QShortcut *activateShortcut =
      new QShortcut(keymap.activate, p_target, [this] { hint(); });
  QShortcut *editShortcut = new QShortcut(keymap.edit, p_target,
                                          [this] { hint(HintMode::Editable); });
  QShortcut *yankShortcut = new QShortcut(keymap.yank, p_target,
                                          [this] { hint(HintMode::Yankable); });
  QShortcut *focusShortcut = new QShortcut(
      keymap.focus, p_target, [this] { hint(HintMode::Focusable); });
  QShortcut *cancelShortcut =
      new QShortcut(keymap.cancel, p_target, [this] { cancel(); });
  shortcuts.append(activateShortcut);
  shortcuts.append(editShortcut);
  shortcuts.append(yankShortcut);
  shortcuts.append(focusShortcut);
  shortcuts.append(cancelShortcut);
}

WindowController::WindowController(QWidget *_target, QObject *parent = nullptr)
    : QObject(parent), p_target(_target) {
  Q_ASSERT(parent);
  initializeShortcuts();
  initializeOverlays();
  Q_ASSERT(p_overlays.length() > 0);
  p_target->installEventFilter(this);
}

WindowController::~WindowController() {
  for (auto &overlay : p_overlays) {
    delete overlay;
  }
  for (auto shortcut : shortcuts) {
    delete shortcut;
  }
}

/*
 * Filter QKeyEvents before the receiver widgets get a change to act on them.
 * Called from the application eventFilter.
 */
bool WindowController::earlyKeyEventFilter(QKeyEvent *kev) {
  logInfo << __PRETTY_FUNCTION__ << kev;
  auto type = kev->type();
  if (type == QEvent::KeyPress) {
    if (mode == ControllerMode::Hint) {
      if (kev->key() == Qt::Key_Escape) {
        cancel();
        return true;
      } else if (kev->key() == Qt::Key_Return) {
        acceptCurrent();
        return true;
      } else if (kev->key() == Qt::Key_Backspace) {
        popKey();
        return true;
      } else if (kev->key() == Qt::Key_Tab) {
        activeOverlay()->nextHint(true);
        return true;
      } else if (kev->key() == Qt::Key_Backtab) {
        activeOverlay()->nextHint(false);
        return true;
      } else if ((kev->key() >= 'A') && (kev->key() <= 'Z')) {
        // std::isalpha doesn't work here
        logDebug << kev->key();
        pushKey(kev->key());
        return true;
      }
    }
  }
  return false;
}

bool WindowController::eventFilter(QObject *obj, QEvent *ev) {
  /*
   * auto type = ev->type();
   * if (type == QEvent::KeyPress) {
   *   QKeyEvent *kev = static_cast<QKeyEvent *>(ev);
   *   if (mode == ControllerMode::Hint) {
   *     if (kev->key() == Qt::Key_Escape) {
   *       cancel();
   *     } else if (kev->key() == Qt::Key_Enter) {
   *       acceptCurrent();
   *     } else if (kev->key() == Qt::Key_Backspace) {
   *       popKey();
   *     } else if (kev->key() == Qt::Key_Tab) {
   *       activeOverlay()->nextHint(kev->modifiers() & Qt::Modifier::SHIFT);
   *     } else if (std::isalpha(kev->key())) {
   *       pushKey(kev->key());
   *     }
   *   }
   * }
   * return false;
   */
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

// As with tryAttachToWindow, can be called to attach to existing
// overlayable widget, or in response to one being created.
void WindowController::tryAttachController(QWidget *target) {
  // TODO 09/09/20 psacawa: add checks for both cases
  if (target != nullptr) {
    addOverlay(target);
  }
}

void WindowController::addOverlay(QWidget *target) {
  p_overlays.append(new Overlay(target));
  connect(this, &WindowController::destroyed, this, [this](QObject *obj) {
    Overlay *overlay = qobject_cast<Overlay *>(obj);
    this->removeOverlay(overlay, true);
  });
}

void WindowController::removeOverlay(Overlay *overlay, bool fromSignal) {
  if (!fromSignal) {
    delete overlay;
  }
  int idx = p_overlays.indexOf(overlay);
  if (idx >= 0) {
    p_overlays.removeAt(idx);
  }
}

void WindowController::hint(HintMode hintMode) {
  qInfo() << __PRETTY_FUNCTION__;
  if (!(mode == ControllerMode::Normal)) {
    logWarning << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  logDebug << __PRETTY_FUNCTION__;
  hintBuffer = "";
  QList<QWidget *> hintables = this->getHintables(hintMode);
  HintGenerator hintStringGenerator(Controller::settings.hintChars,
                                    hintables.length());
  for (auto widget : hintables) {
    string hintStr = *hintStringGenerator;
    logDebug << "Hinting " << widget << " with " << hintStr.c_str();
    mainOverlay()->addHint(QString::fromStdString(*hintStringGenerator),
                           widget);
    hintStringGenerator++;
  }
  mainOverlay()->setVisible(true);
  mainOverlay()->resetSelection();
  mode = ControllerMode::Hint;

  // In hint mode, just disable shortcuts. Need something cleaner?
  for (auto sc : shortcuts)
    sc->setEnabled(false);

  currentHintMode = hintMode;
}
void WindowController::acceptCurrent() {
  QWidget *current = activeOverlay()->selectedWidget();
  if (current == nullptr) {
    logWarning << "Active overlay has no selected hint";
    return;
  }
  accept(current);
}

void WindowController::accept(QWidget *widget) {
  if (!(mode == ControllerMode::Hint)) {
    logWarning << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  logInfo << "Accepted " << widget << "in" << currentHintMode;
  switch (currentHintMode) {
  case Activatable: {
    if (auto button = qobject_cast<QAbstractButton *>(widget)) {
      button->click();
    } else {
      logWarning << "Don't know how to activate " << widget << "in"
                 << currentHintMode;
    }
    break;
  }
  case Focusable:
    // TODO 24/08/20 psacawa: set focussed GUI state for button...
    widget->setFocus();
    if (QAbstractButton *button = qobject_cast<QAbstractButton *>(widget)) {
      button->setDown(true);
    }
  case Editable: {
    widget->setFocus();
    /*
     * if (const auto lineEdit = qobject_cast<QLineEdit *>(widget)) {
     * } else {
     *   else_block
     * }
     */
    break;
  }
  case Yankable: {
    if (QLabel *label = qobject_cast<QLabel *>(widget)) {
      QClipboard *clipboard = qApp->clipboard();
      QString text = label->text();
      clipboard->setText(text);
    } else {
      logWarning << "unimplemented";
    }
    break;
  }
  default:
    Q_UNREACHABLE();
    break;
  }
  cancel();
}

void WindowController::pushKey(char ch) {
  if (!(mode == ControllerMode::Hint)) {
    logWarning << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  hintBuffer += ch;
  int numVisibleHints = activeOverlay()->updateHints(hintBuffer);
  if (numVisibleHints == 1 && Controller::settings.autoAcceptUniqueHint) {
    acceptCurrent();
  } else if (numVisibleHints == 0) {
    cancel();
  }
}

void WindowController::popKey() {
  if (!(mode == ControllerMode::Hint)) {
    logWarning << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  hintBuffer.remove(hintBuffer.length() - 1, 1);
  activeOverlay()->updateHints(hintBuffer);
}

QWidget *WindowController::target() {
  return dynamic_cast<QWidget *>(this->p_target);
}

void WindowController::cancel() {
  if (!(mode == ControllerMode::Hint)) {
    logWarning << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  logDebug << __PRETTY_FUNCTION__;
  for (auto overlay : p_overlays) {
    overlay->clear();
    overlay->hide();
  }
  mode = ControllerMode::Normal;
  for (auto sc : shortcuts)
    sc->setEnabled(true);
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

  // TODO 02/08/20 psacawa: dla się bez tego kopiowania?
  // return QList<QWidget *>(hintables.begin(), hintables.end());
  return QList<QWidget *>(hintables.begin(), hintables.end());
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
  // TODO 22/07/20 psacawa: fix pre-inc
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
  logInfo << "CSS:" << ret;
  return ret;
}

} // namespace Tetradactyl
