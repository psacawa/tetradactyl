// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QDebug>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QSet>
#include <QTimer>
#include <QWindow>

// #include <QtCore/private/qhooks_p.h>

namespace Tetradactyl {

#define objProbe ObjectProbe::instance()

// A singleton binding the Qt hooks to the information necessary for probing
// created QObjects for QWindow etc. The class may well by superfluous.
class ObjectProbe : public QObject {
  Q_OBJECT
public:
  ObjectProbe();
  ObjectProbe(ObjectProbe &) = delete;
  ObjectProbe &operator=(ObjectProbe &) = delete;

  // hooks must be static for ABI reasons (no hidden "this" pointer param)
  static void addQObjectCallback(QObject *obj);
  static void removeQObjectCallback(QObject *obj);
  static void startupCallback();

  void afterAppInitialization();
  void attachControllerToWindows();

  void processCreatedObjects();
  static bool earlyFilterObject(QObject *obj);
  bool interestedObject(QObject *obj);
  bool isClientWidget(QWidget *w);

  static ObjectProbe *instance();

signals:
  void objectCreated(QObject *obj, QPrivateSignal);

private:
  QList<QObject *> createdObjects;
  static ObjectProbe *self;
  QTimer timer;
  // controls access to objectsBeingCreated
  QMutex mutex;
  QQueue<QObject *> objectsBeingCreated;
  QSet<QWidget *> clientAppWidgets;
};

inline bool ObjectProbe::isClientWidget(QWidget *w) {
  return clientAppWidgets.contains(w);
}
} // namespace Tetradactyl
