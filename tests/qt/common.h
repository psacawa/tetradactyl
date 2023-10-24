// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QApplication>
#include <QSignalSpy>
#include <QWidget>

#include <qt/controller.h>

#define pressKey(...) QTest::keyClick(qApp->focusWidget(), __VA_ARGS__)
#define pressKeys(...) QTest::keyClicks(qApp->focusWidget(), __VA_ARGS__)

namespace Tetradactyl {

class QtBaseTest : public QObject {
public:
  QtBaseTest() {}
  ~QtBaseTest() {}

protected:
  void init() {
    Controller::createController();
    controller = Controller::instance();
    settings = &controller->settings;
    windowController = controller->windows().value(0, nullptr);
    if (windowController == nullptr)
      QFAIL("No WindowController found in Controller. Did you create the demo "
            "main widget?");

    overlay = windowController->mainOverlay();

    modeChangedSpy =
        new QSignalSpy(windowController, &WindowController::modeChanged);
    hintedSpy = new QSignalSpy(windowController, &WindowController::hinted);
    acceptedSpy = new QSignalSpy(windowController, &WindowController::accepted);
    cancelledSpy =
        new QSignalSpy(windowController, &WindowController::cancelled);
    hintingFinishedSpy =
        new QSignalSpy(windowController, &WindowController::hintingFinished);
  }

  const Controller *controller;
  WindowController *windowController;
  ControllerSettings *settings;
  Overlay *overlay;

  QSignalSpy *modeChangedSpy, *hintedSpy, *acceptedSpy, *cancelledSpy,
      *hintingFinishedSpy;
};

void waitForWindowActiveOrFail(QWidget *);

} // namespace Tetradactyl

// default to offscreen rendering
static void __attribute__((constructor)) setupDefaultQtPlatform() {
  if (qgetenv("QT_QPA_PLATFORM") == "")
    qputenv("QT_QPA_PLATFORM", "offscreen");
}
