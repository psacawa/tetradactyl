// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QApplication>
#include <QDebug>
#include <QList>
#include <QLoggingCategory>
#include <QMenuBar>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QWindow>

#include <QtCore/private/qhooks_p.h>

#include <csignal>

#include <backward.hpp>

#include <common/backtrace.h>

#include "controller.h"
#include "logging.h"
#include "probe.h"
#include "version.h"

LOGGING_CATEGORY_COLOR("tetradactyl.probe", Qt::red);

using QHooks::AddQObjectCallback;
using QHooks::HookIndex;
using QHooks::RemoveQObjectCallback;
using QHooks::StartupCallback;

static StartupCallback nextStartupCallback;
static AddQObjectCallback nextAddQObjectCallback;
static RemoveQObjectCallback nextRemoveQObjectCallback;

#if DEBUG
backward::SignalHandling sh({
    // don't trace SIGABRT/SIGIOT since that corresponds to failed asserts
    // SIGABRT,
    SIGBUS,
    SIGFPE,
    SIGILL,
    // SIGIOT,
    SIGQUIT,
    SIGSEGV,
    SIGSYS,
    // SIGTRAP,
    SIGXCPU,
    SIGXFSZ,
});
#endif

namespace Tetradactyl {

void __attribute__((constructor)) init() {

  // hooks
  nextStartupCallback =
      reinterpret_cast<StartupCallback>(qtHookData[HookIndex::Startup]);
  nextAddQObjectCallback =
      reinterpret_cast<AddQObjectCallback>(qtHookData[HookIndex::AddQObject]);
  qtHookData[HookIndex::Startup] =
      reinterpret_cast<unsigned long long>(ObjectProbe::startupCallback);
  qtHookData[HookIndex::AddQObject] =
      reinterpret_cast<unsigned long long>(ObjectProbe::addQObjectCallback);
  qtHookData[HookIndex::RemoveQObject] =
      reinterpret_cast<unsigned long long>(ObjectProbe::removeQObjectCallback);

  // Don't pass LD_PRELOAD to eventual subprocesses.
  // setenv("LD_PRELOAD", "", 1);
}

ObjectProbe::ObjectProbe() {
  Q_ASSERT(instance() == nullptr);
  self = this;
  timer.setSingleShot(true);
  timer.setInterval(0);
  connect(&timer, &QTimer::timeout, this, &ObjectProbe::processCreatedObjects);
}

void ObjectProbe::startupCallback() {
  qInstallMessageHandler(colorMessageHandler);
  self = new ObjectProbe;

  QTimer::singleShot(0, []() { Controller::createController(); });
  if (nextStartupCallback) {
    nextStartupCallback();
  }
}

void ObjectProbe::addQObjectCallback(QObject *obj) {
  // We are called from the base object constructor QObject::QObject. Therefore
  // anything that relies on virtual functions ,such as
  //
  // QWindow *win = qobject_cast<QWindow *>(obj);
  //
  // will not work. The aproach is instead to add the QObject* to a queue for
  // post-processing after creation. A QTimer is used to asynchronously connect
  // object creation to the postprocessing.

  if (earlyFilterObject(obj))
    return;

  {
    TETRA_MUTEX_LOCKER locker(&self->mutex);
    instance()->objectsBeingCreated.push_back(obj);
    if (!instance()->timer.isActive()) {
      instance()->timer.start();
    }
  }
  if (nextAddQObjectCallback) {
    nextAddQObjectCallback(obj);
  }
}

void ObjectProbe::removeQObjectCallback(QObject *obj) {
  if (earlyFilterObject(obj))
    return;

  // if object is queued to be processed in objectsBeingCreated, we must remove
  // it, else segfault later
  {
    TETRA_MUTEX_LOCKER locker(&instance()->mutex);
    int index = self->objectsBeingCreated.indexOf(obj);
    if (index >= 0)
      self->objectsBeingCreated.removeAt(index);
  }
  if (nextRemoveQObjectCallback) {
    nextRemoveQObjectCallback(obj);
  }
}

// Filter partially constructed object from entering queue here.  For
// performance.
bool ObjectProbe::earlyFilterObject(QObject *obj) {
  // only interested in UI thread
  if (self == nullptr)
    return true;
  if (QThread::currentThread() != self->thread())
    return true;
  return false;
}

// Is Tetradactyl interested in this object? For the most part, at this point,
// we can't definitively tell if the object is intendend to be a window. If so,
// then it may be that parent == nullptr, and that it will be a
// tetradactylWindow will only be apparent upon calling show(),
bool ObjectProbe::interestedObject(QObject *obj) {
  static QList<const QMetaObject *> interestedMetaObjects = {
      &QMenuBar::staticMetaObject};
  const QMetaObject *thisMO = obj->metaObject();
  for (auto mo : interestedMetaObjects) {
    if (thisMO->inherits(mo)) {
      return true;
    }
  }
  return false;
}

void ObjectProbe::processCreatedObjects() {
  Q_ASSERT(QThread::currentThread() == self->thread());
  {
    logDebug << "Processing created objects";
    TETRA_MUTEX_LOCKER locker(&mutex);
    for (auto obj : objectsBeingCreated) {
      if (interestedObject(obj)) {
        // can't use QDebug with QObjects here
        fprintf(stderr, "Client created object %s(%p)\n",
                obj->metaObject()->className(), (void *)obj);
        emit objectCreated(obj, QPrivateSignal());
      }
    }
    objectsBeingCreated.clear();
  }
}

inline ObjectProbe *ObjectProbe::instance() { return self; }
ObjectProbe *ObjectProbe::self = nullptr;
} // namespace Tetradactyl
