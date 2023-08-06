#include <QDebug>
#include <QObject>
#include <Qt>

#include <cctype>
#include <cstdio>
#include <cstring>

#include "controller.h"
#include "filter.h"

using Tetradactyl::Controller;
using Tetradactyl::ControllerKeymap;

void install_event_filter_rec(QObject *root) {
  root->installEventFilter(new KeyboardEventFilter);
  for (auto child : root->children()) {
    install_event_filter_rec(child);
  }
}

KeyboardEventFilter::KeyboardEventFilter(QObject *obj,
                                         Tetradactyl::Controller *_controller)
    : controller(_controller) {
  if (obj) {
    owner = obj;
    obj->installEventFilter(this);
  }
}

bool KeyboardEventFilter::eventFilter(QObject *obj, QEvent *ev = nullptr) {
  if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::KeyRelease) {
    if (ev->type() == QEvent::KeyPress) {
      QKeyEvent *kev = static_cast<QKeyEvent *>(ev);
      Tetradactyl::ControllerKeymap &keymap = controller->settings.keymap;
      int key = kev->key();
      if (controller->state == Controller::State::Normal) {
        if (key == keymap.hintKey) {
          this->controller->hint();
          return true;
        }
      } else if (controller->state == Controller::State::Hint) {
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
