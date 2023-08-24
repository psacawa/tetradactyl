#include <QApplication>
#include <QDebug>
#include <QList>
#include <QLoggingCategory>
#include <QObject>
#include <QTextStream>
#include <QTimer>
#include <QWindow>

#include <QtCore/private/qhooks_p.h>

#include <cstdio>

#include "controller.h"
#include "inject.h"

using QHooks::AddQObjectCallback;
using QHooks::HookIndex;
using QHooks::RemoveQObjectCallback;
using QHooks::StartupCallback;
using Tetradactyl::Probe;

#define lcThis tetradactylInject
Q_LOGGING_CATEGORY(tetradactylInject, "tetradactyl.inject");

static StartupCallback nextStartupCallback;
static AddQObjectCallback nextAddQObjectCallback;
static RemoveQObjectCallback nextRemoveQObjectCallback;

void __attribute__((constructor)) initHooks() {

  nextStartupCallback =
      reinterpret_cast<StartupCallback>(qtHookData[HookIndex::Startup]);
  nextAddQObjectCallback =
      reinterpret_cast<AddQObjectCallback>(qtHookData[HookIndex::AddQObject]);

  qtHookData[HookIndex::Startup] =
      reinterpret_cast<unsigned long long>(Probe::startupCallback);
  qtHookData[HookIndex::AddQObject] =
      reinterpret_cast<unsigned long long>(Probe::addQObjectCallback);
  qtHookData[HookIndex::RemoveQObject] =
      reinterpret_cast<unsigned long long>(Probe::removeQObjectCallback);
}

Probe::Probe() {
  Q_ASSERT(instance() == nullptr);
  p_instance = this;
  timer.setSingleShot(true);
}

void Probe::startupCallback() {

  QTimer::singleShot(0, []() { Probe::afterAppInitialization(); });
  if (nextStartupCallback) {
    nextStartupCallback();
  }
}

void Probe::addQObjectCallback(QObject *obj) {
  // We are called from the base object constructor QObject::QObject. Therefore
  // anything that relies on virtual functions ,such as
  //
  // QWindow *win = qobject_cast<QWindow *>(obj);
  //
  // will not work. The aproach is instead to add the QObject* to a queue for
  // post-processing after creation. A QTimer is used to asynchronously connect
  // object creation to the postprocessing.

  // QList<QObject *> &queue = instance()->createdObjects;
  // QTimer &timer = instance()->timer;

  // queue.push_back(obj);
  // if (!timer.isActive()) {
  //   timer.start(0);
  // }

  if (nextAddQObjectCallback) {
    nextAddQObjectCallback(obj);
  }
}

void Probe::removeQObjectCallback(QObject *obj) {
  if (nextRemoveQObjectCallback) {
    nextRemoveQObjectCallback(obj);
  }
}

void Probe::afterAppInitialization() {
  attachControllerToWindows();

  // Initially defocus input widgets. This assumes the tol-level widget is not
  // e.g. QLineEdit
  for (auto widget : qApp->topLevelWidgets()) {
    widget->setFocus();
  }
}

void Probe::attachControllerToWindows() {
  QList<QWindow *> windows = qApp->allWindows();
  for (auto win : windows) {
    qCInfo(lcThis) << "Attaching Tetradactyl to " << win;
    new Tetradactyl::Controller(win);
  }
}

Probe *Probe::instance() { return p_instance; }
Probe *Probe::p_instance = nullptr;
