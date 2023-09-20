// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QAction>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMetaObject>
#include <QShortcut>
#include <QWidget>

#include <vector>

using std::vector;

namespace Tetradactyl {
class Controller;

class KeyboardEventFilter : public QObject {
  Q_OBJECT
public:
  KeyboardEventFilter(QObject *obj = nullptr,
                      Tetradactyl::Controller *controller = nullptr);

protected:
  bool eventFilter(QObject *obj, QEvent *ev) override;

private:
  // vector <QKeySequence>
  static vector<const QMetaObject *> inputMetaObjects;
  bool inputWidgetFocussed();
  QObject *owner = nullptr;
  Tetradactyl::Controller *controller = nullptr;
};

class PrintFilter : public QObject {
  Q_OBJECT
public:
  PrintFilter() {}
  virtual ~PrintFilter() {}

private:
  bool on = false;
  QList<const QMetaObject *> interestedMetaObjects = {
      // &QAction::staticMetaObject, &QShortcut::staticMetaObject,
      // &QWidget::staticMetaObject
      &QMainWindow::staticMetaObject};
  QList<const QMetaObject *> disinterestedMetaObjects = {};
  QList<QEvent::Type> interestedEventTypes = {
      // QEvent::KeyPress,
      // QEvent::Shortcut
      QEvent::Resize
      // , QEvent::Show,
      // QEvent::ShowToParent
  };
  QList<QEvent::Type> disinterestedEventTypes = {};

  bool interestedInMetaObject(QObject *obj);

  bool interestedInEventType(const QEvent &event);

  bool eventFilter(QObject *obj, QEvent *ev);
};

} // namespace Tetradactyl
