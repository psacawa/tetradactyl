#include <QDebug>
#include <QObject>
#include <QWindow>

#include <QtCore/private/qhooks_p.h>

#include "controller.h"

#include <cstdio>
#include <qobject.h>
#include <qvariant.h>

using QHooks::AddQObjectCallback;
using QHooks::HookIndex;
using QHooks::RemoveQObjectCallback;
using QHooks::StartupCallback;
using Tetradactyl::Controller;

void startupCallback();
void addQObjectCallback(QObject *obj);
void removeQObjectCallback(QObject *obj);

static StartupCallback nextStartupCallback;
static AddQObjectCallback nextAddQObjectCallback;
static RemoveQObjectCallback nextRemoveQObjectCallback;

void __attribute__((constructor)) init_hooks() {
  printf("%s\n", __PRETTY_FUNCTION__);

  nextStartupCallback =
      reinterpret_cast<StartupCallback>(qtHookData[HookIndex::Startup]);
  nextAddQObjectCallback =
      reinterpret_cast<AddQObjectCallback>(qtHookData[HookIndex::AddQObject]);

  qtHookData[HookIndex::Startup] =
      reinterpret_cast<unsigned long long>(startupCallback);
  qtHookData[HookIndex::AddQObject] =
      reinterpret_cast<unsigned long long>(addQObjectCallback);
}

void startupCallback() {
  printf("%s\n", __PRETTY_FUNCTION__);
  if (nextStartupCallback) {
    nextStartupCallback();
  }
}

void addQObjectCallback(QObject *obj) {
  // We are called from the base object constructor QObject::QObject.
  // Thereform anything that relies on virtual functions ,such as qobject_cast
  // does not work
  QWindow *win = qobject_cast<QWindow *>(obj);
  // if (obj->inherits("QWindow")) {
  // if (obj->metaObject()->inherits(&QWindow::staticMetaObject)) {
  if (QWindow *win = qobject_cast<QWindow *>(obj)) {
    qInfo() << "QWindow added " << obj;
    Controller *controller = new Controller(win);
  }
  if (nextAddQObjectCallback) {
    nextAddQObjectCallback(obj);
  }
}

void removeQObjectCallback(QObject *obj) {
  printf("%s\n", __PRETTY_FUNCTION__);
  if (nextRemoveQObjectCallback) {
    nextRemoveQObjectCallback(obj);
  }
}
