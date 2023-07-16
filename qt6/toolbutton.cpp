#include <QEvent>
#include <QToolButton>

#include <algorithm>
#include <iostream>
#include <memory>

#include "common.h"
#include "qlist.h"
#include "qnamespace.h"

using std::cout;
using std::unique_ptr;

#define DECLARE_ORIGINAL(member) decltype(member) me = (member)
#define SET_TO_ORIGINAL_FUNCTION(x)                                            \
  (x) = (decltype(x))dlsym(RTLD_NEXT, thisFunctionGetMangledName());

QToolButton::QToolButton(QWidget *w) {
  // printf("%s\n", __PRETTY_FUNCTION__);
  void (*original_function)(QToolButton *, QWidget *);
  SET_TO_ORIGINAL_FUNCTION(original_function);
  original_function(this, w);
}

bool QToolButton::event(QEvent *e) {
  // printf("%s \n", __PRETTY_FUNCTION__);
  bool (*original_function)(QToolButton *, QEvent *);
  SET_TO_ORIGINAL_FUNCTION(original_function);
  return original_function(this, e);
}

void QToolButton::mousePressEvent(QMouseEvent *e) {
  // printf("%s \n", __PRETTY_FUNCTION__);

  QWidget *w = this->window();
  auto children =
      w->findChildren<QObject>("", Qt::FindChildrenRecursively);

  // print_tree(w);

  cout << children.size() << " children found\n";
  for (int i = 0; i != children.size(); ++i) {
    cout << children.at(i).objectName().toStdString() << '\n';
  }

  void (*original_function)(QToolButton *, QMouseEvent *);
  SET_TO_ORIGINAL_FUNCTION(original_function);
  return original_function(this, e);
}

void QToolButton::mouseReleaseEvent(QMouseEvent *e) {
  // printf("%s \n", __PRETTY_FUNCTION__);
  void (*original_function)(QToolButton *, QMouseEvent *);
  SET_TO_ORIGINAL_FUNCTION(original_function);
  return original_function(this, e);
}
