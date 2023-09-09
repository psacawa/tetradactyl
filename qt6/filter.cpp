// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QLoggingCategory>
#include <QObject>
#include <QTextEdit>
#include <Qt>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <qloggingcategory.h>
#include <qobject.h>

#include "controller.h"
#include "filter.h"
#include "logging.h"

LOGGING_CATEGORY_COLOR("tetradactyl.filter", Qt::green);

namespace Tetradactyl {

vector<const QMetaObject *> KeyboardEventFilter::inputMetaObjects = {
    &QLineEdit::staticMetaObject, &QTextEdit::staticMetaObject};

KeyboardEventFilter::KeyboardEventFilter(QObject *obj,
                                         Tetradactyl::Controller *_controller)
    : controller(_controller) {
  if (obj) {
    owner = obj;
    obj->installEventFilter(this);
  }
}

bool KeyboardEventFilter::inputWidgetFocussed() {
  QWidget *focusWidget = qApp->focusWidget();
  for (auto mo : inputMetaObjects) {
    if (mo->cast(focusWidget) != nullptr) {
      return true;
    }
  }
  return false;
}

bool KeyboardEventFilter::eventFilter(QObject *obj, QEvent *ev = nullptr) {
  if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::KeyRelease) {
    if (ev->type() == QEvent::KeyPress) {

      QKeyEvent *kev = static_cast<QKeyEvent *>(ev);
      // Tetradactyl::ControllerKeymap &keymap = controller->settings.keymap;
      QKeyCombination kc = QKeyCombination::fromCombined(kev->key());

      // current technique to let input widgets get input is to avoid filtering
      // the input if the current focussed widget is considered an input widget.
      // Only  <esc> is captured to escape the focus

      if (inputWidgetFocussed()) {
        logInfo << "Input widget is focussed. Passing keypress" << kev;
        if (kc == QKeyCombination(Qt::Key_Escape)) {
          // is there a better default for focus to escape inputs?
          // controller->host()->setFocus();
          return true;
        }
        return false;
      }
    }
  }
  return false;
}

// In the special case that  there are interested and disinterested items (a
// user error), an interested match is enough to match
bool PrintFilter::interestedInMetaObject(QObject *obj) {
  if (interestedMetaObjects.length() != 0) {
    const QMetaObject *metaObj = obj->metaObject();
    for (auto mo : interestedMetaObjects) {
      if (metaObj->inherits(mo)) {
        return true;
      }
    }
  }
  if (disinterestedMetaObjects.length() != 0) {
    const QMetaObject *metaObj = obj->metaObject();
    for (auto mo : disinterestedMetaObjects) {
      if (metaObj->inherits(mo)) {
        return false;
      }
    }
  }
  if (interestedMetaObjects.length() == 0 &&
      disinterestedMetaObjects.length() == 0) {
    return true;
  }
  return false;
}

bool PrintFilter::interestedInEventType(const QEvent &event) {
  QEvent::Type type = event.type();
  if (interestedEventTypes.indexOf(type) >= 0)
    return true;

  if (disinterestedEventTypes.length() != 0) {
    if (disinterestedEventTypes.indexOf(type) >= 0)
      return false;
  }

  if (interestedEventTypes.length() == 0 &&
      disinterestedEventTypes.length() == 0) {
    return true;
  }
  return false;
}

bool PrintFilter::eventFilter(QObject *obj, QEvent *ev) {
  if (interestedInEventType(*ev) && interestedInMetaObject(obj)) {
    logInfo << ev << obj;
  }
  return false;
}

} // namespace Tetradactyl
