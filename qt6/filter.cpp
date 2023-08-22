#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QLoggingCategory>
#include <QObject>
#include <QTextEdit>
#include <Qt>

#include <cctype>
#include <cstdio>
#include <cstring>
#include <qobject.h>

#include "controller.h"
#include "filter.h"

using Tetradactyl::Controller;
using HintMode = Tetradactyl::Controller::HintMode;
using Tetradactyl::ControllerKeymap;
using ControllerMode = Tetradactyl::Controller::ControllerMode;

#define THIS_LOG tetradactylFilter
Q_LOGGING_CATEGORY(tetradactylFilter, "tetradactyl.filter");

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
      Tetradactyl::ControllerKeymap &keymap = controller->settings.keymap;
      int key = kev->key();

      // current technique to let input widgets get input is to avoid filtering
      // the input if the current focussed widget is considered an input widget.
      // Only  <esc> is captured to escape the focus

      if (inputWidgetFocussed()) {
        qDebug() << "Input widget is focussed. Passing keypress" << kev;
        if (key == Qt::Key_Escape) {
          // is there a better default for focus to escape inputs?
          controller->myToplevelWidget()->setFocus();
          return true;
        }
        return false;
      }

      if (controller->mode == ControllerMode::Normal) {
        if (key == keymap.hintKey) {
          this->controller->hint();
          return true;
        } else if (key == Qt::Key_G) {
          this->controller->hint(HintMode::Editable);
          return true;
        }
      } else if (controller->mode == Controller::ControllerMode::Hint) {
        if (key == keymap.cancelKey) {
          this->controller->cancel();
          return true;
        } else if (key == Qt::Key_Backspace) {
          this->controller->popKey();
          return true;
        } else if (isalpha(key)) {
          // TODO 02/08/20 psacawa: finish this
          this->controller->pushKey(toupper(key));
          return true;
        }
      }
    }
  }
  return false;
}
} // namespace Tetradactyl
