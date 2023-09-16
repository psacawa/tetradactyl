// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QSignalSpy>
#include <QWidget>

#include <qt/controller.h>

namespace Tetradactyl {

class QtBaseTest : public QObject {
public:
  QtBaseTest() {}
  ~QtBaseTest() {}

protected:
  void init() {
    qInfo() << __PRETTY_FUNCTION__;
    Controller::createController();
    controller = Controller::instance();
    windowController = controller->windows().at(0);
    overlay = windowController->mainOverlay();

    hintedSpy = new QSignalSpy(windowController, &WindowController::hinted);
    acceptedSpy = new QSignalSpy(windowController, &WindowController::accepted);
    cancelledSpy =
        new QSignalSpy(windowController, &WindowController::cancelled);
  }

  const Controller *controller;
  WindowController *windowController;
  Overlay *overlay;

  QSignalSpy *hintedSpy, *acceptedSpy, *cancelledSpy;
};

void waitForWindowActiveOrFail(QWidget *);

} // namespace Tetradactyl

// default to offscreen rendering
static void __attribute__((constructor)) setupDefaultQtPlatform() {
  if (qgetenv("QT_QPA_PLATFORM") == "")
    qputenv("QT_QPA_PLATFORM", "offscreen");
}
