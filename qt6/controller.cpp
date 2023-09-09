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
    {Focusable, {&QWidget::staticMetaObject}},
    {Yankable, {&QLabel::staticMetaObject}}};

struct ControllerSettings Controller::settings {
  .hintChars = "ASDFJKL",
  .keymap = {.activate = QKeySequence(Qt::Key_F),
             .cancel = QKeySequence(Qt::Key_Escape),
             .edit = QKeySequence(Qt::Key_G, Qt::Key_I),
             .focus = QKeySequence(Qt::Key_Semicolon),
             .yank = QKeySequence(Qt::Key_Y),
             .activateMenu = QKeySequence(Qt::Key_M),
             .activateContext = QKeySequence(Qt::Key_G, Qt::Key_C),
             .upScroll = QKeySequence(Qt::Key_K),
             .downScroll = QKeySequence(Qt::Key_J)}
};

QString Controller::stylesheet = "*{ background-color: blue; }";

Controller::Controller(QWidget *parent) : QObject(parent) {
  Q_ASSERT(self == nullptr);
  Controller::stylesheet = fetchStylesheet();
  qApp->setStyleSheet(Controller::stylesheet);

  qApp->installEventFilter(new Tetradactyl::PrintFilter);
  qApp->installEventFilter(this);
}

Controller::~Controller() {}

void Controller::createController() {
  logInfo << "Creating Tetradactyl controller";
  self = new Controller();

  self->attachToExistingWindows();

  // Initially defocus input widgets. This assumes the tol-level widget is not
  // e.g. QLineEdit
  for (auto widget : qApp->topLevelWidgets()) {
    widget->setFocus();
  }
}

void Controller::attachToExistingWindows() {
  QList<QWidget *> topelevelWidgets = qApp->topLevelWidgets();
  for (auto &widget : topelevelWidgets) {
    // QTimer::singleShot(0, [widget] { self->tryAttachToWindow(widget); });
    tryAttachToWindow(widget);
  }
}

// Check if the widget is a window and not a popup. If so, create a
// WindowController
void Controller::tryAttachToWindow(QWidget *widget) {
  // Make sure no to attach WindowController to things with the popup bit
  // set, such as  QMenu (in menu bar and context menu)
  if (widget->isWindow() &&
      !(widget->windowFlags() & WindowType::Popup & ~WindowType::Window)) {
    logInfo << "Attaching Tetradactyl to" << widget;
    self->windowControllers.append(
        new Tetradactyl::WindowController(widget, this));
    // }
  }
}

// TODO 09/09/20 psacawa: test it works
bool Controller::eventFilter(QObject *obj, QEvent *ev) {
  QEvent::Type type = ev->type();
  QWidget *widget = qobject_cast<QWidget *>(obj);
  if (type == QEvent::Show && widget != nullptr) {
    tryAttachToWindow(widget);
  }
  return false;
}

void Controller::routeObject(QObject *obj) {
  const QMetaObject *mo = obj->metaObject();
  // TODO 09/09/20 psacawa: add WindowController overlay or wait for show
  // event as necessary
}

Controller *Controller::self = nullptr;

void WindowController::initializeShortcuts() {
  ControllerKeymap &keymap = controller->settings.keymap;
  QShortcut __attribute__((unused)) *activateShortcut =
      new QShortcut(keymap.activate, target, [this] { hint(); });
  QShortcut __attribute__((unused)) *focusShortcut = new QShortcut(
      keymap.focus, target, [this] { hint(HintMode::Focusable); });
  QShortcut __attribute__((unused)) *editShortcut =
      new QShortcut(keymap.edit, target, [this] { hint(HintMode::Editable); });
  QShortcut __attribute__((unused)) *yankShortcut =
      new QShortcut(keymap.yank, target, [this] { hint(HintMode::Yankable); });
  QShortcut __attribute__((unused)) *cancelShortcut =
      new QShortcut(keymap.cancel, target, [this] { cancel(); });
}

WindowController::WindowController(QWidget *_target, QObject *parent = nullptr)
    : QObject(parent), target(_target) {
  Q_ASSERT(parent);
  initializeShortcuts();
  initializeOverlays();
  target->installEventFilter(this);
}

bool WindowController::eventFilter(QObject *obj, QEvent *ev) {
  auto type = ev->type();
  if (type == QEvent::KeyPress) {
    QKeyEvent *kev = static_cast<QKeyEvent *>(ev);
    if (mode == ControllerMode::Hint && kev->key() != Qt::Key_Escape) {
      pushKey(kev->key());
    }
  }
  return false;
}

// Current strategy: just search for the overlayable classes under the target
void WindowController::initializeOverlays() {
  QList<QWidget *> widgets = target->findChildren<QWidget *>();
  for (auto widget : qApp->topLevelWidgets()) {
    if (isDescendantOf(widget, target)) {
      tryAttachController(widget);
    }
  }
}

// As with tryAttachToWindow, can be called to attach to existing overlayable
// widget, or in response to one being created.
void WindowController::tryAttachController(QWidget *widget) {
  // TODO 09/09/20 psacawa: add checks for both cases
  overlays.append(new Overlay(widget));
}

void WindowController::hint(HintMode hintMode) {
  if (!(mode == ControllerMode::Normal)) {
    logInfo << __PRETTY_FUNCTION__ << "from" << mode;
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
    HintLabel *hint =
        new HintLabel(QString::fromStdString(*hintStringGenerator), widget);
    hint->parentWidget()->setProperty("selected", QVariant(true));
    hint->show();
    hints.push_back(hint);
    hintStringGenerator++;
  }
  selectedHint = hints.begin();
  mode = ControllerMode::Hint;
  currentHintMode = hintMode;
}

void WindowController::acceptCurrent() {}

void WindowController::accept(QWidget *widget) {
  if (!(mode == ControllerMode::Hint)) {
    logInfo << __PRETTY_FUNCTION__ << "from" << mode;
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
    logInfo << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  hintBuffer += ch;
  filterHints();

  // if selectedHint hidden, set new one
  // if ((*selectedHint)->isHidden()) {
  //   selectedHint = visibleHints.begin();
  // }
}

QWidget *WindowController::host() {
  return dynamic_cast<QWidget *>(this->parent());
  Q_UNREACHABLE();
}

void WindowController::filterHints() {
  vector<QWidget *> invisibleHints;
  visibleHints = {};
  std::partition_copy(hints.begin(), hints.end(),
                      std::back_inserter(visibleHints),
                      std::back_inserter(invisibleHints), [&](HintLabel *hint) {
                        return hint->text().startsWith(hintBuffer.c_str());
                      });
  for (auto widget : visibleHints) {
    widget->show();
  }
  for (auto widget : invisibleHints) {
    widget->hide();
  }
  if (visibleHints.size() == 1) {
    accept(qobject_cast<QWidget *>(visibleHints[0]->parent()));
  } else if (visibleHints.size() == 0) {
    cancel();
  }
}

void WindowController::cancel() {
  if (!(mode == ControllerMode::Hint)) {
    logInfo << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  logDebug << __PRETTY_FUNCTION__;
  for (auto hint : hints) {
    delete hint;
  }
  hints = {};
  visibleHints = {};
  mode = ControllerMode::Normal;
}

// For now the  hinting proceed only by type: QObjects whose meta-object are
// right get hinted. However, to support clients with more complex handling of
// events, etc., we will want a more dynamic approach.
QList<QWidget *> WindowController::getHintables(HintMode hintMode) {
  QList<QWidget *> descendants = target->findChildren<QWidget *>();
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
  // find the minimum lengthOfHints to be able to generate enough hint strings
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
  return ret;
}

} // namespace Tetradactyl
