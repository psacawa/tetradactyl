// #include <QKeyEvent>
#include <Qt>

#include "common.h"
#include <qabstractbutton.h>
#include <qpushbutton.h>

typedef void (*keyPressEventFunc)(QAbstractButton *button, QKeyEvent *e);
// typedef void (*keyPressEventFunc)(QKeyEvent *e);

void QAbstractButton::keyPressEvent(QKeyEvent *e) {
  static void (*original_function)(void *, QKeyEvent *e);
  original_function = (decltype(original_function))dlsym(
      RTLD_NEXT, thisFunctionGetMangledName());
  return (original_function)(this, e);
}

void QAbstractButton::keyReleaseEvent(QKeyEvent *e) {
  static void (*original_function)(void *, QKeyEvent *e);
  original_function = (decltype(original_function))dlsym(
      RTLD_NEXT, thisFunctionGetMangledName());
  return (original_function)(this, e);
}
