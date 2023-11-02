// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QApplication>
#include <QList>
#include <QMetaEnum>
#include <QMetaObject>
#include <QString>

#include "common.h"
#include "controller.h"
#include "hint.h"
#include "overlay.h"

namespace Tetradactyl {

static QSet<const QMetaObject *> tetradactylMetaObjects = {
    &HintLabel::staticMetaObject, &Overlay::staticMetaObject,
    &Controller::staticMetaObject, &WindowController::staticMetaObject};

bool isTetradactylMetaObject(const QMetaObject *mo) {
  return tetradactylMetaObjects.contains(mo);
}

bool isTetradactylObject(QObject *obj) {
  const QMetaObject *mo = obj->metaObject();
  return isTetradactylMetaObject(mo);
}

bool isDescendantOf(QObject *descendant, QObject *ancestor) {
  for (; descendant != nullptr; descendant = descendant->parent()) {
    if (descendant == ancestor)
      return true;
  }
  return false;
}

// widget can have Tetradactyl::WindowController attached to it: be a window and
// not a popup
bool isTetradactylWindow(QWidget *w) {
  QList<Qt::WindowType> windowTypes = {Qt::WindowType::Window,
                                       Qt::WindowType::Dialog};
  return w->isWindow() && windowTypes.contains(w->windowType());
}

// Widget can have an Tetradactyl::Overlay attached to it; essentially
// tetradactyl windows + popups
bool isTetradactylOverlayable(QWidget *w) {
  QList<Qt::WindowType> windowTypes = {
      Qt::WindowType::Window, Qt::WindowType::Dialog, Qt::WindowType::Popup};
  return windowTypes.contains(w->windowType());
}

// @pre: win != nullptr
// @post return != nullptr (?)
QWidget *getToplevelWidgetForWindow(QWindow *win) {
  Q_ASSERT(win != nullptr);
  for (auto w : qApp->topLevelWidgets()) {
    if (w->windowHandle() == win) {
      return w;
    }
  }
  Q_UNREACHABLE();
}

const char promptStylesheet[] =
    "* { background-color: #444; color: white; font-family: Monospace; "
    " padding: 1px; }";

} // namespace Tetradactyl
