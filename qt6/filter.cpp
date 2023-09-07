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

#define lcThis tetradactylFilter
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
      QKeyCombination kc = QKeyCombination::fromCombined(kev->key());

      // current technique to let input widgets get input is to avoid filtering
      // the input if the current focussed widget is considered an input widget.
      // Only  <esc> is captured to escape the focus

      if (inputWidgetFocussed()) {
        qCDebug(lcThis) << "Input widget is focussed. Passing keypress" << kev;
        if (kc == QKeyCombination(Qt::Key_Escape)) {
          // is there a better default for focus to escape inputs?
          controller->myToplevelWidget()->setFocus();
          return true;
        }
        return false;
      }

      if (controller->mode == Controller::ControllerMode::Normal) {
        // TODO 22/08/20 psacawa: really handle shortcuts input buffering
        if (kc == keymap.activate[0]) {
          this->controller->hint();
          return true;
        } else if (kc == keymap.focus[0]) {
          this->controller->hint(HintMode::Focusable);
          return true;
        } else if (kc == keymap.yank[0]) {
          this->controller->hint(HintMode::Yankable);
          return true;
        } else if (kc == keymap.edit[0]) {
          this->controller->hint(HintMode::Editable);
          return true;
        } else if (kc == keymap.upScroll[0]) {
          this->controller->hint(HintMode::Editable);
          return true;
        }
      } else if (controller->mode == Controller::ControllerMode::Hint) {
        if (kc == keymap.cancel[0]) {
          this->controller->cancel();
          return true;
        } else if (kc == QKeyCombination(Qt::Key_Backspace)) {
          this->controller->popKey();
          return true;
        } else if (kc.keyboardModifiers() == Qt::NoModifier &&
                   isalpha(kc.key())) {
          // TODO 02/08/20 psacawa: finish this
          this->controller->pushKey(toupper(kc.key()));
          return true;
        }
      }
    }
  }
  return false;
}
} // namespace Tetradactyl
